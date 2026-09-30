# Tài liệu thiết kế định danh tác vụ

Tài liệu này trình bày các khái niệm và định nghĩa liên quan đến việc định danh các tác vụ (task) trong dự án μEDP. Việc định danh tác vụ giúp định danh và phân phối tác vụ một cách hiệu quả, đảm bảo rằng các tác vụ được quản lý và thực thi đúng cách trong hệ thống.

## Nguyên tắc định danh tác vụ

1. Tránh đặt tên ID theo phiên bản vì dự án μEDP có thể được nâng cấp và thay đổi theo thời gian. Thay vào đó, hãy sử dụng các tên ID mang tính chất ổn định và không thay đổi theo phiên bản.
2. Tránh đặt tên ID theo số thứ tự tuyến tính, vì điều này có thể dẫn đến sự nhầm lẫn và khó quản lý khi số lượng tác vụ tăng lên. Thay vào đó, hãy sử dụng các tên ID mang tính chất mô tả và dễ hiểu.

## Thiết kế ID cho các tác vụ

Các ID của các tác vụ trong μEDP nên được thiết kế theo các nguyên tắc sau:

```design
[PREFIX] - [INDEX]
```

Tiền tố đại diện cho phạm vi ảnh hưởng của Task. Nó bất biến cho dù Task đó bị đẩy sang bất kỳ phiên bản nào.

Chỉ số (INDEX) là một số nguyên duy nhất để phân biệt các Task trong cùng một phạm vi ảnh hưởng.

## Bộ tiền tố

1. `[CORE - XXX]` dành cho mã nguồn lõi của μEDP, bao gồm các thành phần như PLD/μE-LS, S-LnF APE, PLTF, và các thành phần khác liên quan đến lõi của hệ thống.
2. `[PLTF - XXX]` dành cho các thành phần liên quan đến khung kiểm thử cục bộ (Portable Local Test Framework), bao gồm các thành phần như testobj, BST, và các thành phần khác liên quan đến kiểm thử.
3. `[DOCS - XXX]` dành cho các tài liệu liên quan đến dự án μEDP, bao gồm các tài liệu như hướng dẫn sử dụng, tài liệu thiết kế, và các tài liệu khác liên quan đến dự án.
4. `[TEST - XXX]` dành cho các tác vụ liên quan đến kiểm thử, bao gồm các tác vụ như kiểm thử đơn vị, kiểm thử tích hợp, và các tác vụ khác liên quan đến kiểm thử.
5. `[NETW - XXX]` dành cho các tác vụ liên quan đến mạng, bao gồm các tác vụ như cấu hình mạng, kiểm tra kết nối, và các tác vụ khác liên quan đến mạng.
6. `[REPO - XXX]` dành cho các tác vụ liên quan đến kho lưu trữ mã nguồn, bao gồm các tác vụ như quản lý nhánh, hợp nhất mã nguồn, và các tác vụ khác liên quan đến kho lưu trữ.

## Directive cho Comment Anchors

Sử dụng `@!` và `<+` để đánh dấu các comment anchors trong mã nguồn. Các comment anchors này giúp định danh và phân phối các tác vụ một cách hiệu quả, đảm bảo rằng các tác vụ được quản lý và thực thi đúng cách trong hệ thống.
