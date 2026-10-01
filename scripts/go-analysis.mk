# Use the installed toolchain; normal checks must not silently download Go.
GOTOOLCHAIN ?= local
export GOTOOLCHAIN
GO_FIX_FLAGS ?=
# Analysis tool versions copied from fsledger. Installation is explicit/networked.
GO_PACKAGES ?= ./...
GO_MODULE ?= github.com/Take-a-Chef/exim-observer-protocol
GO_TOOLS_DIR := $(CURDIR)/.tools/bin
GOLANGCI_VERSION := v2.13.2
GOVULNCHECK_VERSION := v1.7.0
NILAWAY_VERSION := v0.0.0-20260802165852-32ec3a0e8a41

.PHONY: tools-go tools-check-go lint-config lint-go nilaway vuln fmt-go
tools-go:
	GOWORK=off GOPROXY=https://proxy.golang.org,direct GOSUMDB=sum.golang.org GOBIN=$(GO_TOOLS_DIR) go install github.com/golangci/golangci-lint/v2/cmd/golangci-lint@$(GOLANGCI_VERSION)
	GOWORK=off GOPROXY=https://proxy.golang.org,direct GOSUMDB=sum.golang.org GOBIN=$(GO_TOOLS_DIR) go install golang.org/x/vuln/cmd/govulncheck@$(GOVULNCHECK_VERSION)
	GOWORK=off GOPROXY=https://proxy.golang.org,direct GOSUMDB=sum.golang.org GOBIN=$(GO_TOOLS_DIR) go install go.uber.org/nilaway/cmd/nilaway@$(NILAWAY_VERSION)
tools-check-go:
	$(GO_TOOLS_DIR)/golangci-lint version | grep -F 'version $(GOLANGCI_VERSION:v%=%) '
	go version -m $(GO_TOOLS_DIR)/nilaway | grep -F '$(NILAWAY_VERSION)'
	go version -m $(GO_TOOLS_DIR)/govulncheck | grep -F '$(GOVULNCHECK_VERSION)'
lint-config: tools-check-go
	$(GO_TOOLS_DIR)/golangci-lint config verify
lint-go: lint-config
	GOPROXY=off GOSUMDB=off $(GO_TOOLS_DIR)/golangci-lint run $(GO_PACKAGES)
nilaway: tools-check-go
	GOPROXY=off GOSUMDB=off $(GO_TOOLS_DIR)/nilaway -include-pkgs=$(GO_MODULE) -exclude-test-files=false $(GO_PACKAGES)
# Explicit online security audit; not part of the offline make check contract.
vuln: tools-check-go
	$(GO_TOOLS_DIR)/govulncheck -test $(GO_PACKAGES)
fmt-go:
	$(GO_TOOLS_DIR)/golangci-lint fmt $(GO_PACKAGES)

.PHONY: tools
tools: tools-go

.PHONY: fix-go fix-go-check
# Explicit modernization; formatter changes are intentional in this target.
fix-go:
	GOPROXY=off GOSUMDB=off go fix $(GO_FIX_FLAGS) $(GO_PACKAGES)
	$(MAKE) fmt-go
# Read-only check: go fix -diff fails when a modernizer suggests changes.
fix-go-check:
	GOPROXY=off GOSUMDB=off go fix -diff $(GO_FIX_FLAGS) $(GO_PACKAGES)
check: fix-go-check
