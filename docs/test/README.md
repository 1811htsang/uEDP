# Test Case Optimization for Propositional Logic and FSMs

- Status: Guidance
- Scope: Requirement coverage with a practical number of test cases
- Techniques: Propositional logic, MC/DC, decision tables, pairwise testing, and finite state machine (FSM) coverage

## 1. Basic Test-Case Rules for Propositional Operators

For $n$ independent Boolean conditions, exhaustive truth-table testing requires $2^n$ combinations. The following rules provide smaller, risk-based test sets while preserving the required logic coverage.

### 1.1. Negation

Two test cases are required: one with the operand `True` and one with the operand `False`. The expected result must be the exact inverse of the input.

### 1.2. AND

`A AND B` is `True` only when every operand is `True`. For $n$ operands, exhaustive testing requires $2^n$ cases; MC/DC requires $n + 1$ cases.

| ID | A | B | Expected result | Purpose |
| --- | --- | --- | --- | --- |
| AND-01 | True | True | True | All conditions satisfied |
| AND-02 | False | True | False | A independently changes the decision |
| AND-03 | True | False | False | B independently changes the decision |

### 1.3. OR

`A OR B` is `True` when at least one operand is `True`. For $n$ operands, MC/DC requires $n + 1$ cases.

| ID | A | B | Expected result | Purpose |
| --- | --- | --- | --- | --- |
| OR-01 | False | False | False | All conditions are false |
| OR-02 | True | False | True | A independently changes the decision |
| OR-03 | False | True | True | B independently changes the decision |

### 1.4. Implication

Implication is equivalent to $A \rightarrow B = \neg A \lor B$. The only false case is `A = True, B = False`.

| ID | A | B | Expected result | Purpose |
| --- | --- | --- | --- | --- |
| IMP-01 | True | True | True | True premise with true consequence |
| IMP-02 | True | False | False | Premise is violated |
| IMP-03 | False | True | True | False premise makes implication true |
| IMP-04 | False | False | True | Confirms both false-premise cases |

### 1.5. Equivalence

Equivalence is `True` when both operands have the same value. The complete two-input truth table requires four cases.

| ID | A | B | Expected result |
| --- | --- | --- | --- |
| EQ-01 | True | True | True |
| EQ-02 | True | False | False |
| EQ-03 | False | True | False |
| EQ-04 | False | False | True |

### 1.6. Exclusive OR (XOR)

`A XOR B` is `True` when exactly one operand is `True`: $A \oplus B = (A \lor B) \land \neg(A \land B)$.

- Exhaustive testing for two operands requires four cases.
- MC/DC can be demonstrated with three cases, for example `00`, `10`, and `01`; the fourth case is recommended to complete the truth table.

| ID | A | B | Expected result | Purpose |
| --- | --- | --- | --- | --- |
| XOR-01 | False | False | False | Neither condition is true |
| XOR-02 | True | False | True | A independently changes the decision |
| XOR-03 | False | True | True | B independently changes the decision |
| XOR-04 | True | True | False | Both conditions are true, so exclusivity is enforced |

## 2. Propositional-Logic Optimization

Use the following techniques to prevent combinatorial growth when requirements contain complex Boolean constraints.

### 2.1. Boolean Algebra and Karnaugh-Map Simplification

1. Express the business rule as a Boolean formula.
2. Simplify it with Boolean algebra, De Morgan's laws, absorption, factoring, or a Karnaugh map.
3. Generate tests from the simplified expression and retain traceability to the original requirement.

### 2.2. Remove Mutually Exclusive and Impossible Combinations

Exclude combinations that cannot exist in the application domain. For example, an automotive licensing rule need not test `Age < 18 AND DriverLicense = Valid` when that combination is prohibited by the domain. Such exclusions must be documented as constraints rather than silently removed.

### 2.3. Use Condensed Decision Tables

Merge rules with the same action when one or more conditions do not affect the result. Use `-` for a don't-care condition. This reduces the number of columns without losing meaningful requirement coverage.

### 2.4. Use Pairwise (All-Pairs) Testing

When many mostly independent variables make exhaustive testing impractical, ensure that every pair of input values occurs at least once. Pairwise testing reduces the test set substantially while preserving coverage of many two-factor interaction faults. It does not replace MC/DC when condition-level independence is required.

## 3. FSM Transition-Coverage Optimization

An FSM consists of states ($S$), events ($E$), transitions ($T$), and guard conditions. Select the coverage level according to risk.

### 3.1. 0-Switch / All-Transitions Coverage

Execute every valid transition $S_i \xrightarrow{E} S_j$ at least once. This is the default cost-effective level for most software systems.

### 3.2. 1-Switch Coverage

Execute every valid pair of consecutive transitions, $S_i \rightarrow S_j \rightarrow S_k$. Use this level for workflows where event order is safety- or business-critical, such as banking transactions, medical devices, or multi-level approvals.

### 3.3. Invalid-Transition Testing

Select high-risk invalid events and representative states instead of testing every event in every state. Verify the defined rejection, error reporting, state retention, and recovery behavior.

### 3.4. Integrate Guards with Transitions

A guarded transition occurs only when both the event and the guard are satisfied:

$$\text{Transition} = \text{Event} \land \text{Guard Condition}$$

Optimize and test the guard as a propositional expression before using it in the FSM transition matrix.

## 4. Step-by-Step Workflow

### Step 1: Model the system

List all states and events. Express every transition guard as a standard Boolean expression.

### Step 2: Optimize guard conditions

Create a decision table or Karnaugh map for complex guards. Reduce the rules and derive MC/DC test pairs where applicable.

### Step 3: Build the optimized transition matrix

Place states on the rows and events on the columns. Record valid transitions, guards, expected actions, and invalid-event behavior.

### Step 4: Generate path-based test cases

Combine transitions into end-to-end scenarios so that one scenario covers several transitions. Add focused negative cases for high-risk invalid events and guard failures.

## 5. Worked Example: ATM Withdrawal

### 5.1. Requirement

The ATM has these states: `Idle`, `Authenticated`, `AmountSelected`, and `Dispensing`.

The withdrawal guard is:

$$C = A \land B \land D$$

where:

- $A$: withdrawal amount is less than or equal to the account balance;
- $B$: withdrawal amount is less than or equal to the daily transaction limit;
- $D$: the ATM has enough cash or permits partial withdrawal, $D = E \lor F$.

### 5.2. MC/DC Test Set

The full expression has four primitive conditions ($A$, $B$, $E$, and $F$), which would require $2^4 = 16$ exhaustive combinations. Testing the decision at the guard level uses four representative cases for $A$, $B$, and the evaluated guard $D$.

| ID | A | B | D | Expected guard C | Expected system action |
| --- | --- | --- | --- | --- | --- |
| ATM-01 | True | True | True | True | Transition to `Dispensing` |
| ATM-02 | False | True | True | False | Report insufficient balance |
| ATM-03 | True | False | True | False | Report daily-limit violation |
| ATM-04 | True | True | False | False | Report insufficient ATM cash |

Because $D = E \lor F$, add the OR test set from Section 1.3 when coverage of the internal guard operands is required.

### 5.3. FSM Scenarios

**Success path:**

$$\texttt{Idle} \xrightarrow{\texttt{Insert Card and PIN}} \texttt{Authenticated} \xrightarrow{\texttt{Select Amount}} \texttt{AmountSelected} \xrightarrow{C=\texttt{True}} \texttt{Dispensing} \xrightarrow{\texttt{Take Cash}} \texttt{Idle}$$

**Guard-failure path:**

$$\texttt{Authenticated} \xrightarrow{\texttt{Select Amount}} \texttt{AmountSelected} \xrightarrow{C=\texttt{False}} \texttt{Authenticated}$$

The ATM reports the relevant error and allows the authenticated user to select another amount.

**Invalid-event path:**

$$\texttt{Idle} \xrightarrow{\texttt{Select Amount}} \texttt{Reject\ or\ Ignore}$$

Selecting an amount before authentication must not advance the FSM.

### 5.4. Expected Optimization Result

| Metric | Full combinatorial and complete FSM testing | MC/DC and 0-switch testing |
| --- | --- | --- |
| Logic test cases | 16 cases for four primitive conditions | Four guard-level cases, plus XOR/OR operand cases when required |
| FSM test cases | More than 16 state-event combinations | Two to three end-to-end scenarios |
| Total test effort | More than 32 cases | Approximately six focused scenarios, adjusted for required operand coverage |
| Coverage | 100% mathematical combinations | MC/DC guard coverage and 100% valid-transition coverage |
