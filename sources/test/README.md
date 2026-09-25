# DOC

As required of merging and adding folder for testobj, this README file is created to provide information about the test directory and its contents.

## Directory

Currently, the `test` directory is split into two subdir including `testobj` and `deprecated`.

The `testobj` subdir is used to store test objects compliant with the test framework. These test objects are used for testing purposes and can be replaced or updated as needed. Therefore, naming convention for this directory will be add versioning for tracking.

For example, `testobj-v1.yaml` indicates the first version of the test object, while `testobj-v2.yaml` indicates the second version. This hex versioning allows for easy tracking of changes and updates to the test objects over time. After the versioning reaches `F - 16` e.g. `testobj-v00f.yaml`, the next version will be `testobj-v010.yaml` and so on.

In addition to the type of testobj, it must also be mentioned with follow specification:

- `vir-` prefix indicates that the testobj is used for Linux virtual machine testing.
- `phy-` prefix indicates that the testobj is used for physical embedded machine testing.
- Both `vir-` and `phy-` prefixes can be used together to indicate that the testobj is used for both virtual and physical testing.
- `logic-` prefix indicates that the testobj is used for logic testing, it can be the subprefix for `vir-` or `phy-` prefix.

Meanwhile the `deprecated` subdir contains deprecated test files that are no longer in use, these test files existed long from the beginning of the project and are kept for reference purposes only. The naming convention for this directory will not be in used, but keep the original name of the test files for reference.
