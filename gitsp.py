import os
import re
import subprocess
import tkinter as tk
from datetime import date, datetime, timedelta
from tkinter import filedialog, messagebox, ttk

CREATE_NO_WINDOW = 0x08000000 if os.name == "nt" else 0
SEP = "\x1f"
DATE_FORMAT = "--date=format:%Y-%m-%d %H:%M"
LOG_FORMAT = f"--pretty=format:%H{SEP}%h{SEP}%ad{SEP}%an{SEP}%s"


class GitError(Exception):
    pass


def git_run(repo, *args):
    try:
        result = subprocess.run(
            ["git", "-C", repo, *args],
            capture_output=True,
            creationflags=CREATE_NO_WINDOW,
        )
    except OSError as exc:
        raise GitError(f"Không chạy được git: {exc}") from exc
    return result.returncode, result.stdout, result.stderr


def git_ok(repo, *args):
    code, out, err = git_run(repo, *args)
    if code != 0:
        raise GitError((err or out).decode("utf-8", errors="replace").strip())
    return out


def detect_repo(start):
    try:
        return git_ok(start, "rev-parse", "--show-toplevel").decode("utf-8", errors="replace").strip()
    except GitError:
        return start


def list_branches(repo):
    raw = git_ok(repo, "for-each-ref", "--format=%(refname:short)", "refs/heads", "refs/remotes")
    names = {
        name
        for name in raw.decode("utf-8", errors="replace").splitlines()
        if name and name != "origin" and not name.endswith("/HEAD")
    }
    return sorted(names)


def current_branch(repo):
    return git_ok(repo, "branch", "--show-current").decode("utf-8", errors="replace").strip()


def list_files(repo, branch):
    raw = git_ok(repo, "ls-tree", "-r", "-z", "--name-only", branch)
    return sorted(path for path in raw.decode("utf-8", errors="replace").split("\0") if path)


def copy_files_from_branch(repo, branch, paths):
    for path in paths:
        content = git_ok(repo, "cat-file", "blob", f"{branch}:{path}")
        target = os.path.join(repo, *path.split("/"))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, "wb") as handle:
            handle.write(content)


def read_log(repo, *args):
    raw = git_ok(repo, "log", DATE_FORMAT, LOG_FORMAT, *args).decode("utf-8", errors="replace")
    commits = []
    for line in raw.splitlines():
        full, short, when, author, subject = line.split(SEP, 4)
        commits.append((full, short, when, author, subject))
    return commits


def commits_in_period(repo, branch, since, until):
    return read_log(repo, f"--since={since} 00:00:00", f"--until={until} 23:59:59", branch)


def commits_missing_in_target(repo, source, target):
    return read_log(repo, "--no-merges", "--reverse", "--cherry-pick", "--right-only", f"{target}...{source}")


def working_tree_dirty(repo):
    return bool(git_ok(repo, "status", "--porcelain", "--untracked-files=no").strip())


def cherry_pick_in_progress(repo):
    return git_run(repo, "rev-parse", "-q", "--verify", "CHERRY_PICK_HEAD")[0] == 0


COMMIT_TYPES = (
    ("feat", "Thêm năng lực hoặc hành vi mới"),
    ("fix", "Sửa lỗi, sai so với yêu cầu, regression"),
    ("docs", "Chỉ cập nhật tài liệu"),
    ("chore", "Bảo trì, tooling, cấu hình, build"),
    ("struct", "Tổ chức lại cấu trúc tài liệu hoặc thư mục"),
    ("test", "Thêm hoặc sửa test case"),
    ("reft", "Refactor, không đổi hành vi"),
    ("sub-feat", "Commit trên branch sub-feat theo version"),
    ("app", "Thay đổi trong app hoặc testobj riêng"),
    ("release", "Thay đổi trên branch release"),
)


def build_commit_message(ctype, scope, name, subject):
    head = ctype
    if scope.strip():
        head += f"({scope.strip()})"
    if name.strip():
        head += f"[{name.strip()}]"
    return f"{head}: {subject.strip().rstrip('.') or '<subject>'}"


COMMENT_TAGS = (
    ("NOTE", "Ghi chú bổ sung cho task hoặc đoạn phía trên"),
    ("STATUS", "Tiến độ hiện tại, kết quả đã đạt hoặc lý do đóng task"),
    ("CRITICAL", "Ràng buộc, cảnh báo hoặc thay đổi phạm vi quan trọng"),
    ("REVIEW", "Nội dung cần review hoặc thống nhất với người khác"),
    ("DEPRECATED", "Nội dung đã bỏ, giữ lại để tham chiếu"),
    ("FIXME", "Vấn đề tồn đọng cần sửa"),
    ("LINK", "Liên kết tới file hoặc dòng code liên quan"),
)

TICKET_PREFIXES = ("ARCH", "CORE", "DOCS", "PLTF", "REPO", "TEST")
TODO_RELPATH = ("docs", "to-do.md")


def build_comment(tag, text, inline):
    body = text.strip() or "<nội dung>"
    if inline:
        return f"//{tag} - {body}"
    return f"<!-- {tag}\n{body}\n-->"


def build_ticket(done, deprecated, epic, prefix, number, text):
    marker = "DEPRECATED" if deprecated else "TID"
    if epic.strip():
        marker += f"[epic={epic.strip()}]"
    number = f"{int(number):03d}" if number.strip().isdigit() else (number.strip() or "NNN")
    mark = "x" if done else " "
    return f"- [{mark}] @! {marker} {prefix}-{number} <+ {text.strip() or '<mô tả>'}"


def next_ticket_number(todo_path, prefix):
    try:
        with open(todo_path, encoding="utf-8") as handle:
            content = handle.read()
    except OSError as exc:
        raise GitError(f"Không đọc được {todo_path}: {exc}") from exc
    numbers = [int(num) for pfx, num in re.findall(r"\b([A-Z]+)-(\d{3})\b", content) if pfx == prefix]
    return f"{max(numbers, default=0) + 1:03d}"


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Git Branch Tool")
        self.geometry("1000x720")
        self.minsize(840, 580)

        script_dir = os.path.dirname(os.path.abspath(__file__))
        self.repo_var = tk.StringVar(value=detect_repo(script_dir))
        self.current_var = tk.StringVar(value="-")
        self.source_var = tk.StringVar()
        self.period_branch_var = tk.StringVar()
        self.filter_var = tk.StringVar()
        self.since_var = tk.StringVar(value=(date.today() - timedelta(days=7)).isoformat())
        self.until_var = tk.StringVar(value=date.today().isoformat())
        self.status_var = tk.StringVar(value="Sẵn sàng")
        self.current_name = ""
        self.all_files = []
        self.visible_files = []
        self.ctype_var = tk.StringVar(value="feat")
        self.commit_scope_var = tk.StringVar()
        self.commit_name_var = tk.StringVar()
        self.commit_subject_var = tk.StringVar()
        self.commit_preview_var = tk.StringVar()
        for var in (self.ctype_var, self.commit_scope_var, self.commit_name_var, self.commit_subject_var):
            var.trace_add("write", lambda *_: self.update_commit_preview())
        self.doc_mode_var = tk.StringVar(value="note")
        self.note_tag_var = tk.StringVar(value="NOTE")
        self.note_inline_var = tk.BooleanVar(value=False)
        self.note_desc_var = tk.StringVar()
        self.ticket_done_var = tk.BooleanVar(value=False)
        self.ticket_deprecated_var = tk.BooleanVar(value=False)
        self.ticket_epic_var = tk.StringVar()
        self.ticket_prefix_var = tk.StringVar(value="DOCS")
        self.ticket_number_var = tk.StringVar()
        self.ticket_text_var = tk.StringVar()
        self.doc_preview_var = tk.StringVar()
        for var in (
            self.note_tag_var,
            self.note_inline_var,
            self.ticket_done_var,
            self.ticket_deprecated_var,
            self.ticket_epic_var,
            self.ticket_prefix_var,
            self.ticket_number_var,
            self.ticket_text_var,
        ):
            var.trace_add("write", lambda *_: self.update_doc_preview())

        self._build_log()
        self._build_header()
        self._build_tabs()
        self.update_commit_preview()
        self.guarded(self.refresh_branches)

    def _build_log(self):
        bottom = ttk.Frame(self, padding=(8, 0, 8, 8))
        bottom.pack(fill="x", side="bottom")
        self.log_text = tk.Text(bottom, height=8, wrap="word", state="disabled")
        self.log_text.pack(fill="both", expand=True)
        ttk.Label(bottom, textvariable=self.status_var).pack(anchor="w", pady=(4, 0))

    def _build_header(self):
        header = ttk.Frame(self, padding=8)
        header.pack(fill="x")
        header.columnconfigure(1, weight=1)

        ttk.Label(header, text="Repo:").grid(row=0, column=0, sticky="w")
        ttk.Entry(header, textvariable=self.repo_var).grid(row=0, column=1, sticky="ew", padx=4)
        ttk.Button(header, text="Chọn...", command=lambda: self.guarded(self.choose_repo)).grid(row=0, column=2)
        ttk.Button(header, text="Làm mới branch", command=lambda: self.guarded(self.refresh_branches)).grid(
            row=0, column=3, padx=(4, 0)
        )

        ttk.Label(header, text="Branch hiện tại (B):").grid(row=1, column=0, sticky="w", pady=(6, 0))
        ttk.Label(header, textvariable=self.current_var, font=("Segoe UI", 10, "bold")).grid(
            row=1, column=1, sticky="w", pady=(6, 0)
        )

        ttk.Label(header, text="Branch nguồn (A):").grid(row=2, column=0, sticky="w", pady=(6, 0))
        self.source_combo = ttk.Combobox(header, textvariable=self.source_var, state="readonly")
        self.source_combo.grid(row=2, column=1, sticky="ew", padx=4, pady=(6, 0))
        self.source_combo.bind("<<ComboboxSelected>>", lambda _event: self.guarded(self.load_files))

    def _build_tabs(self):
        notebook = ttk.Notebook(self)
        notebook.pack(fill="both", expand=True, padx=8, pady=4)
        self._build_files_tab(notebook)
        self._build_period_tab(notebook)
        self._build_cherry_tab(notebook)
        self._build_commit_tab(notebook)
        self._build_doc_tab(notebook)

    def _build_files_tab(self, notebook):
        tab = ttk.Frame(notebook, padding=8)
        notebook.add(tab, text="Lấy file từ A")

        top = ttk.Frame(tab)
        top.pack(fill="x")
        ttk.Label(top, text="Lọc đường dẫn:").pack(side="left")
        ttk.Entry(top, textvariable=self.filter_var).pack(side="left", fill="x", expand=True, padx=4)
        ttk.Button(top, text="Tải file của A", command=lambda: self.guarded(self.load_files)).pack(side="left")

        list_frame = ttk.Frame(tab)
        list_frame.pack(fill="both", expand=True, pady=6)
        scroll = ttk.Scrollbar(list_frame, orient="vertical")
        self.file_list = tk.Listbox(
            list_frame, selectmode="extended", yscrollcommand=scroll.set, exportselection=False
        )
        scroll.config(command=self.file_list.yview)
        self.file_list.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        self.filter_var.trace_add("write", lambda *_: self.render_files())

        ttk.Button(
            tab,
            text="Lấy file đã chọn từ A vào working tree (B)",
            command=lambda: self.guarded(self.copy_selected_files),
        ).pack(anchor="e")

    def _build_period_tab(self, notebook):
        tab = ttk.Frame(notebook, padding=8)
        notebook.add(tab, text="Commit theo thời gian")

        top = ttk.Frame(tab)
        top.pack(fill="x")
        ttk.Label(top, text="Branch:").pack(side="left")
        self.period_combo = ttk.Combobox(
            top, textvariable=self.period_branch_var, state="readonly", width=34
        )
        self.period_combo.pack(side="left", padx=4)
        ttk.Label(top, text="Từ (YYYY-MM-DD):").pack(side="left", padx=(12, 0))
        ttk.Entry(top, textvariable=self.since_var, width=12).pack(side="left", padx=4)
        ttk.Label(top, text="Đến (YYYY-MM-DD):").pack(side="left", padx=(12, 0))
        ttk.Entry(top, textvariable=self.until_var, width=12).pack(side="left", padx=4)
        ttk.Button(top, text="Lấy commit", command=lambda: self.guarded(self.load_period_commits)).pack(
            side="left", padx=(12, 0)
        )

        self.period_tree = self._make_commit_tree(tab)

    def _build_cherry_tab(self, notebook):
        tab = ttk.Frame(notebook, padding=8)
        notebook.add(tab, text="Cherry-pick")

        top = ttk.Frame(tab)
        top.pack(fill="x")
        ttk.Label(top, text="Commit có trong A nhưng chưa có trong B (chọn nhiều bằng Ctrl/Shift):").pack(
            side="left"
        )
        ttk.Button(top, text="Tải danh sách", command=lambda: self.guarded(self.load_cherry_candidates)).pack(
            side="right"
        )

        self.cherry_tree = self._make_commit_tree(tab)
        ttk.Button(
            tab,
            text="Cherry-pick đã chọn vào B (branch hiện tại)",
            command=lambda: self.guarded(self.cherry_pick_selected),
        ).pack(anchor="e", pady=(8, 0))

    def _build_commit_tab(self, notebook):
        tab = ttk.Frame(notebook, padding=8)
        notebook.add(tab, text="Mẫu commit")

        form = ttk.Frame(tab)
        form.pack(fill="x")
        form.columnconfigure(1, weight=1)
        ttk.Label(form, text="Loại:").grid(row=0, column=0, sticky="w")
        ttk.Combobox(
            form,
            textvariable=self.ctype_var,
            values=[code for code, _ in COMMIT_TYPES],
            state="readonly",
            width=12,
        ).grid(row=0, column=1, sticky="w", padx=4)
        ttk.Label(form, text="Scope:").grid(row=1, column=0, sticky="w", pady=(6, 0))
        ttk.Entry(form, textvariable=self.commit_scope_var).grid(row=1, column=1, sticky="ew", padx=4, pady=(6, 0))
        ttk.Label(form, text="Tên [name]:").grid(row=2, column=0, sticky="w", pady=(6, 0))
        ttk.Entry(form, textvariable=self.commit_name_var).grid(row=2, column=1, sticky="ew", padx=4, pady=(6, 0))
        ttk.Label(form, text="Subject:").grid(row=3, column=0, sticky="w", pady=(6, 0))
        ttk.Entry(form, textvariable=self.commit_subject_var).grid(row=3, column=1, sticky="ew", padx=4, pady=(6, 0))

        preview = ttk.Frame(tab)
        preview.pack(fill="x", pady=(10, 0))
        ttk.Label(preview, text="Xem trước:").pack(side="left")
        ttk.Label(preview, textvariable=self.commit_preview_var, font=("Consolas", 10, "bold")).pack(
            side="left", padx=6
        )
        ttk.Button(
            preview,
            text="Copy vào clipboard",
            command=lambda: self.guarded(self.copy_commit_message),
        ).pack(side="right")

        frame = ttk.Frame(tab)
        frame.pack(fill="both", expand=True, pady=(8, 0))
        self.type_tree = ttk.Treeview(
            frame, columns=("type", "desc", "template"), show="headings", selectmode="browse"
        )
        for column, title, width in (
            ("type", "Loại", 90),
            ("desc", "Dùng khi", 300),
            ("template", "Mẫu", 420),
        ):
            self.type_tree.heading(column, text=title)
            self.type_tree.column(column, width=width, stretch=column == "template")
        for code, desc in COMMIT_TYPES:
            self.type_tree.insert(
                "", "end", iid=code, values=(code, desc, f"{code}(<scope>)[<name>]: <subject>")
            )
        scroll = ttk.Scrollbar(frame, orient="vertical", command=self.type_tree.yview)
        self.type_tree.configure(yscrollcommand=scroll.set)
        self.type_tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        self.type_tree.bind("<<TreeviewSelect>>", self.select_commit_type)

    def select_commit_type(self, _event=None):
        selection = self.type_tree.selection()
        if selection:
            self.ctype_var.set(selection[0])

    def update_commit_preview(self):
        self.commit_preview_var.set(
            build_commit_message(
                self.ctype_var.get(),
                self.commit_scope_var.get(),
                self.commit_name_var.get(),
                self.commit_subject_var.get(),
            )
        )

    def copy_commit_message(self):
        if not self.commit_subject_var.get().strip():
            self.log("Nhập subject trước khi copy.")
            return
        message = self.commit_preview_var.get()
        self.clipboard_clear()
        self.clipboard_append(message)
        self.log(f"Đã copy: {message}")

    def _build_doc_tab(self, notebook):
        tab = ttk.Frame(notebook, padding=8)
        notebook.add(tab, text="Ghi chú & ticket")

        mode_row = ttk.Frame(tab)
        mode_row.pack(fill="x")
        ttk.Label(mode_row, text="Chế độ:").pack(side="left")
        for value, label in (("note", "Ghi chú tài liệu"), ("ticket", "Ticket to-do")):
            ttk.Radiobutton(
                mode_row, text=label, value=value, variable=self.doc_mode_var, command=self.switch_doc_mode
            ).pack(side="left", padx=8)

        self.doc_panel = ttk.Frame(tab)
        self.doc_panel.pack(fill="both", expand=True, pady=(8, 0))
        self.note_frame = ttk.Frame(self.doc_panel)
        self.ticket_frame = ttk.Frame(self.doc_panel)
        self._build_note_form(self.note_frame)
        self._build_ticket_form(self.ticket_frame)

        preview = ttk.Frame(tab)
        preview.pack(fill="x", pady=(10, 0))
        ttk.Label(preview, text="Xem trước:").pack(side="left")
        ttk.Label(preview, textvariable=self.doc_preview_var, font=("Consolas", 10, "bold")).pack(
            side="left", padx=6
        )
        ttk.Button(
            preview,
            text="Copy vào clipboard",
            command=lambda: self.guarded(self.copy_doc_text),
        ).pack(side="right")

        self.switch_doc_mode()

    def _build_note_form(self, parent):
        top = ttk.Frame(parent)
        top.pack(fill="x")
        ttk.Label(top, text="Tag:").pack(side="left")
        ttk.Combobox(
            top,
            textvariable=self.note_tag_var,
            values=[code for code, _ in COMMENT_TAGS],
            state="readonly",
            width=12,
        ).pack(side="left", padx=4)
        ttk.Checkbutton(
            top, text="Dạng inline //TAG - ... (bỏ chọn = block <!-- -->)", variable=self.note_inline_var
        ).pack(side="left", padx=12)
        ttk.Label(parent, textvariable=self.note_desc_var, foreground="gray").pack(anchor="w", pady=(6, 0))
        ttk.Label(parent, text="Nội dung (có thể nhiều dòng):").pack(anchor="w", pady=(8, 0))
        self.note_text = tk.Text(parent, height=8, wrap="word")
        self.note_text.pack(fill="both", expand=True)
        self.note_text.bind("<<Modified>>", self._on_note_modified)

    def _build_ticket_form(self, parent):
        form = ttk.Frame(parent)
        form.pack(fill="x")
        form.columnconfigure(1, weight=1)
        ttk.Checkbutton(form, text="Đã hoàn thành [x]", variable=self.ticket_done_var).grid(
            row=0, column=0, columnspan=2, sticky="w"
        )
        ttk.Checkbutton(form, text="DEPRECATED", variable=self.ticket_deprecated_var).grid(
            row=1, column=0, columnspan=2, sticky="w"
        )
        ttk.Label(form, text="Epic:").grid(row=2, column=0, sticky="w", pady=(6, 0))
        ttk.Entry(form, textvariable=self.ticket_epic_var).grid(row=2, column=1, sticky="ew", padx=4, pady=(6, 0))
        ttk.Label(form, text="Prefix:").grid(row=3, column=0, sticky="w", pady=(6, 0))
        prefix_box = ttk.Combobox(
            form, textvariable=self.ticket_prefix_var, values=TICKET_PREFIXES, state="readonly", width=10
        )
        prefix_box.grid(row=3, column=1, sticky="w", padx=4, pady=(6, 0))
        prefix_box.bind("<<ComboboxSelected>>", lambda _event: self.guarded(self.suggest_ticket_number))
        ttk.Label(form, text="Số:").grid(row=4, column=0, sticky="w", pady=(6, 0))
        number_row = ttk.Frame(form)
        number_row.grid(row=4, column=1, sticky="ew", padx=4, pady=(6, 0))
        ttk.Entry(number_row, textvariable=self.ticket_number_var, width=8).pack(side="left")
        ttk.Button(
            number_row,
            text="Gợi ý số tiếp theo",
            command=lambda: self.guarded(self.suggest_ticket_number),
        ).pack(side="left", padx=6)
        ttk.Label(form, text="Mô tả:").grid(row=5, column=0, sticky="w", pady=(6, 0))
        ttk.Entry(form, textvariable=self.ticket_text_var).grid(row=5, column=1, sticky="ew", padx=4, pady=(6, 0))

    def switch_doc_mode(self):
        is_note = self.doc_mode_var.get() == "note"
        self.note_frame.pack_forget()
        self.ticket_frame.pack_forget()
        if is_note:
            self.note_frame.pack(fill="both", expand=True)
        else:
            self.ticket_frame.pack(fill="x")
            if not self.ticket_number_var.get().strip():
                self.guarded(self.suggest_ticket_number)
        self.update_doc_preview()

    def suggest_ticket_number(self):
        prefix = self.ticket_prefix_var.get()
        todo = os.path.join(self.repo(), *TODO_RELPATH)
        number = next_ticket_number(todo, prefix)
        self.ticket_number_var.set(number)
        self.log(f"Số tiếp theo cho {prefix}: {number}")

    def _on_note_modified(self, _event=None):
        if self.note_text.edit_modified():
            self.note_text.edit_modified(False)
            self.update_doc_preview()

    def update_doc_preview(self):
        if self.doc_mode_var.get() == "note":
            tag = self.note_tag_var.get()
            self.note_desc_var.set(dict(COMMENT_TAGS).get(tag, ""))
            self.doc_preview_var.set(
                build_comment(tag, self.note_text.get("1.0", "end"), self.note_inline_var.get())
            )
        else:
            self.doc_preview_var.set(
                build_ticket(
                    self.ticket_done_var.get(),
                    self.ticket_deprecated_var.get(),
                    self.ticket_epic_var.get(),
                    self.ticket_prefix_var.get(),
                    self.ticket_number_var.get(),
                    self.ticket_text_var.get(),
                )
            )

    def copy_doc_text(self):
        if self.doc_mode_var.get() == "note":
            if not self.note_text.get("1.0", "end").strip():
                self.log("Nhập nội dung ghi chú trước khi copy.")
                return
        else:
            if not self.ticket_text_var.get().strip():
                self.log("Nhập mô tả ticket trước khi copy.")
                return
            if not self.ticket_number_var.get().strip().isdigit():
                self.log("Số ticket phải là số, bấm 'Gợi ý số tiếp theo'.")
                return
        text = self.doc_preview_var.get()
        self.clipboard_clear()
        self.clipboard_append(text)
        self.log(f"Đã copy: {text.splitlines()[0]}")

    def _make_commit_tree(self, parent):
        frame = ttk.Frame(parent)
        frame.pack(fill="both", expand=True, pady=(8, 0))
        tree = ttk.Treeview(
            frame, columns=("hash", "date", "author", "subject"), show="headings", selectmode="extended"
        )
        for column, title, width in (
            ("hash", "Hash", 80),
            ("date", "Ngày", 130),
            ("author", "Tác giả", 160),
            ("subject", "Nội dung", 500),
        ):
            tree.heading(column, text=title)
            tree.column(column, width=width, stretch=column == "subject")
        scroll = ttk.Scrollbar(frame, orient="vertical", command=tree.yview)
        tree.configure(yscrollcommand=scroll.set)
        tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="right", fill="y")
        return tree

    def _fill_commit_tree(self, tree, commits):
        tree.delete(*tree.get_children())
        for full, short, when, author, subject in commits:
            tree.insert("", "end", iid=full, values=(short, when, author, subject))

    def log(self, message):
        stamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.configure(state="normal")
        self.log_text.insert("end", f"[{stamp}] {message}\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")
        self.status_var.set(message.splitlines()[0] if message else "")

    def guarded(self, action):
        try:
            action()
        except GitError as exc:
            self.log(f"Lỗi: {exc}")
            messagebox.showerror("Lỗi git", str(exc))

    def repo(self):
        path = self.repo_var.get().strip()
        if not path:
            raise GitError("Chưa chọn repo.")
        return path

    @staticmethod
    def parse_date(text, label):
        try:
            return datetime.strptime(text.strip(), "%Y-%m-%d").date().isoformat()
        except ValueError:
            raise GitError(f"Ngày '{label}' không hợp lệ, định dạng YYYY-MM-DD.")

    def choose_repo(self):
        path = filedialog.askdirectory(
            initialdir=self.repo_var.get() or None, title="Chọn thư mục git repo"
        )
        if path:
            self.repo_var.set(path)
            self.refresh_branches()

    def refresh_branches(self):
        repo = self.repo()
        branches = list_branches(repo)
        self.current_name = current_branch(repo)
        self.current_var.set(self.current_name or "(detached HEAD)")
        self.source_combo.configure(values=branches)
        self.period_combo.configure(values=branches)

        if self.source_var.get() not in branches:
            others = [b for b in branches if b != self.current_name]
            self.source_var.set(others[0] if others else "")
        if self.period_branch_var.get() not in branches:
            self.period_branch_var.set(self.current_name or (branches[0] if branches else ""))

        self.log(f"Đã tải {len(branches)} branch. Branch hiện tại: {self.current_name or '(detached HEAD)'}")
        self.load_files()

    def load_files(self):
        source = self.source_var.get()
        if not source:
            return
        self.all_files = list_files(self.repo(), source)
        self.render_files()
        self.log(f"Đã tải {len(self.all_files)} file từ {source}.")

    def render_files(self):
        keyword = self.filter_var.get().strip().lower()
        self.visible_files = [path for path in self.all_files if keyword in path.lower()]
        self.file_list.delete(0, "end")
        for path in self.visible_files:
            self.file_list.insert("end", path)

    def copy_selected_files(self):
        repo = self.repo()
        source = self.source_var.get()
        if not source:
            raise GitError("Chưa chọn branch nguồn A.")
        selected = [self.visible_files[i] for i in self.file_list.curselection()]
        if not selected:
            raise GitError("Chưa chọn file nào.")

        overwritten = [p for p in selected if os.path.exists(os.path.join(repo, *p.split("/")))]
        if overwritten and not messagebox.askyesno(
            "Xác nhận ghi đè",
            f"{len(overwritten)} file đã tồn tại trong working tree và sẽ bị ghi đè bằng bản từ {source}.\n"
            "Thay đổi chưa commit ở các file này (nếu có) sẽ mất. Tiếp tục?",
        ):
            return

        copy_files_from_branch(repo, source, selected)
        self.log(f"Đã lấy {len(selected)} file từ {source} vào working tree (chưa stage):\n" + "\n".join(selected))

    def load_period_commits(self):
        repo = self.repo()
        branch = self.period_branch_var.get()
        if not branch:
            raise GitError("Chưa chọn branch.")
        since = self.parse_date(self.since_var.get(), "Từ")
        until = self.parse_date(self.until_var.get(), "Đến")
        if since > until:
            raise GitError("Ngày bắt đầu lớn hơn ngày kết thúc.")

        commits = commits_in_period(repo, branch, since, until)
        self._fill_commit_tree(self.period_tree, commits)
        self.log(f"Tìm thấy {len(commits)} commit trên {branch} từ {since} đến {until}.")

    def load_cherry_candidates(self):
        repo = self.repo()
        source = self.source_var.get()
        if not source:
            raise GitError("Chưa chọn branch nguồn A.")
        if not self.current_name:
            raise GitError("Đang ở detached HEAD, hãy checkout một branch trước.")
        if source == self.current_name:
            raise GitError("Branch nguồn A đang trùng với branch hiện tại B.")

        commits = commits_missing_in_target(repo, source, self.current_name)
        self._fill_commit_tree(self.cherry_tree, commits)
        self.log(f"Có {len(commits)} commit trong {source} chưa có trong {self.current_name}.")

    def cherry_pick_selected(self):
        repo = self.repo()
        if not self.current_name:
            raise GitError("Đang ở detached HEAD, hãy checkout một branch trước.")
        selected = set(self.cherry_tree.selection())
        hashes = [iid for iid in self.cherry_tree.get_children() if iid in selected]
        if not hashes:
            raise GitError("Chưa chọn commit nào.")
        if working_tree_dirty(repo):
            raise GitError("Working tree còn thay đổi đã tracked. Hãy commit hoặc stash trước khi cherry-pick.")
        if not messagebox.askyesno(
            "Xác nhận cherry-pick",
            f"Cherry-pick {len(hashes)} commit vào {self.current_name} theo thứ tự cũ → mới?",
        ):
            return

        code, out, err = git_run(repo, "cherry-pick", *hashes)
        output = (out + err).decode("utf-8", errors="replace").strip()
        if code == 0:
            self.log(f"Cherry-pick thành công {len(hashes)} commit vào {self.current_name}.\n{output}")
            self.load_cherry_candidates()
            return

        self.log(f"Cherry-pick lỗi:\n{output}")
        if cherry_pick_in_progress(repo):
            if messagebox.askyesno(
                "Cherry-pick bị dừng",
                "Có conflict. Abort để quay về trạng thái trước khi cherry-pick?\n"
                "(No: tự giải quyết conflict rồi commit hoặc chạy git cherry-pick --continue.)",
            ):
                abort_code, abort_out, abort_err = git_run(repo, "cherry-pick", "--abort")
                if abort_code != 0:
                    raise GitError((abort_err or abort_out).decode("utf-8", errors="replace").strip())
                self.log("Đã abort cherry-pick.")
                self.load_cherry_candidates()
            else:
                messagebox.showinfo("Cherry-pick", "Hãy giải quyết conflict rồi commit hoặc chạy cherry-pick --continue.")
        else:
            messagebox.showerror("Cherry-pick lỗi", output or "Lỗi không xác định.")


if __name__ == "__main__":
    app = App()
    app.guarded(app.refresh_branches) if False else None
    app.mainloop()
