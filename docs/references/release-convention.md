# Software Release Naming Convention

## Purpose

Software release names provide a standardized way to identify:

- The software baseline
- The release maturity level
- The integration iteration
- The customer delivery version
- The relationship between releases

A release name should allow engineers to quickly answer:

- What software line is this from?
- How mature is this build?
- Which release candidate am I testing?
- Which version was delivered to the customer?

## Common Release Terminology

| Abbreviation | Meaning | Description |
| --- | --- | --- |
| BL | Baseline | Frozen integration point containing a defined set of features and bug fixes |
| R | Release | Official version identifier |
| RC | Release Candidate | Candidate build intended for validation before official release |
| INT | Integration | Internal integration build |
| I0/I1/I2 | Integration Iteration | Sequential integration builds |
| HF | Hotfix | Urgent correction after release |
| SP | Service Pack | Collection of fixes delivered after release |
| GA | General Availability | Final customer/public release |
| P0/P1/P2 | Patch Level | Software patch iteration |
| M1/M2/M3 | Milestone | Development milestone |

## Example: BL100_RC41

```asciidoc
BL100_RC41 
│    │ 
│    └── RC41 = Release Candidate #41 
└─────── BL100 = Baseline #100 
```

### Interpretation

- Software belongs to Baseline 100
- Candidate build number 41
- Usually used for internal validation, regression testing, and release qualification

Example workflow:

```asciidoc
Development
↓
BLX
↓
RCY1
RCY2
RCY3
↓
Customer Release
```

## Example: R1435.1_RC_I0

```asciidoc
R1000.10_RC_I56 
│      │  │ 
│      │  └── Integration Build #56
│      └──── Release Candidate 
└────────── Release Version 1000.10
```

### Explanation

| Part | Meaning |
| --- | --- |
| R1435.1 | Software Release Version 1435.1 |
| RC | Release Candidate |
| I0 | First integration iteration |

## Typical Automotive Software Lifecycle

```asciidoc
Feature Development 
│ 
▼ 
Internal Build 
│ 
▼ 
Baseline (BL) 
│ 
▼ 
Release Candidate (RC) 
│ 
Validation 
│ 
▼ 
Customer Release (Rxxx) 
│ 
▼ 
Hotfix / Patch
```

## Recommended Naming Pattern

### Internal Baseline

`BL<BaselineNumber>_RC<BuildNumber>`

### Release Candidate with Integration

`R<ReleaseVersion>_RC_I<Iteration>`

### Official Release

`R<ReleaseVersion>`

### Hotfix Release

`R<ReleaseVersion>_HF<Number>`

### Service Pack

`R<ReleaseVersion>_SP<Number>`

## Practical Reading Examples

| Release Name | Meaning |
| --- | --- |
| BL100_RCAA | Baseline 100, Release Candidate AA |
| BLUX_RC12 | Baseline UX, Release Candidate 12 |
| R1435.1_RC_I0 | Release 1435.1, RC, Integration Build 0 |
| R1435.1_RC_I3 | Release 1435.1, RC, Integration Build 3 |
| R1435.1 | Official Release 1435.1 |
| R1435.1_HF1 | First Hotfix for Release 1435.1 |
| R1435.1_SP1 | First Service Pack for Release 1435.1 |

This convention is commonly used in automotive software because it clearly separates **baseline**, **release candidate maturity**, and **official customer release versions**.
