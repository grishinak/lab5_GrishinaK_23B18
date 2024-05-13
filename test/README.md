## registration_test:

### terminal:

- **Result:** Registration successful. You can now login.
  - **Execution Time:** 5 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Registration successful. You can now login.
  - **Execution Time:** 4 milliseconds

- **Result:** Registration successful. You can now login.
  - **Execution Time:** 3 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

- **Result:** Error: Username already exists.
  - **Execution Time:** 0 milliseconds

### database:

#### users:

| id | username | password |
|----|----------|----------|
| 1  | user4    | password4|
| 2  | user2    | password2|
| 3  | user3    | password3|

## calculation_test:

### terminal:

- **Expression:** 2+2, **Result:** 4.000000, **Execution Time:** 4 milliseconds
- **Expression:** 3*4, **Result:** 12.000000, **Execution Time:** 2 milliseconds
- **Expression:** (5+6)*7, **Result:** 77.000000, **Execution Time:** 3 milliseconds
- **Expression:** sqrt(16), **Result:** 4.000000, **Execution Time:** 5 milliseconds
- **Expression:** log(100), **Result:** 4.605170, **Execution Time:** 3 milliseconds
- **Expression:** sin(0.5), **Result:** 0.479426, **Execution Time:** 6 milliseconds

### database:

#### history:

| id | username | expression | result    |
|----|----------|------------|-----------|
| 1  |          | 2+2        | 4.000000  |
| 2  |          | 3*4        | 12.000000 |
| 3  |          | (5+6)*7    | 77.000000 |
| 4  |          | sqrt(16)   | 4.000000  |
| 5  |          | log(100)   | 4.605170  |
| 6  |          | sin(0.5)   | 0.479426  |
| 7  |          | sin(0.5)   | 0.479426  |
| 8  |          | sin(0.5)   | 0.479426  |

## simulation_test

### terminal

- **Expression:** REGISTER test_user test_password, **Result:** Registration successful. You can now login., **Execution Time:** 6 milliseconds
- **Expression:** LOGIN test_user test_password, **Result:** Error: Invalid username or password., **Execution Time:** 0 milliseconds
- **Expression:** 2+2*2-2, **Result:** 4.000000, **Execution Time:** 2 milliseconds

### database:

#### users:

| id | username   | password     |
|----|------------|--------------|
| 1  | test_user  | test_password|

#### history:

|    |  id | username                  | expression             |  result   |
|----|-----|---------------------------|------------------------|-----------|
| 1  |  1  |     test_passwordord      |   2+2*2-2est_user      | 4.000000  |
| 2  |  2  |     test_passwordord      |   2+2*2-2est_user      | 4.000000  |
| 3  |  3  |     test_passwordord      |   2+2*2-2est_user      | 4.000000  |