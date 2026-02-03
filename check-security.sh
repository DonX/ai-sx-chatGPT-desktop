#!/bin/bash
# Security check script - Verify no API keys in source code

echo "======================================"
echo "Security Check: API Key Detection"
echo "======================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

FOUND_ISSUES=0

# Check 1: Look for OpenAI API key patterns in source files
echo -e "${YELLOW}[1/5] Checking for hardcoded API keys in source files...${NC}"
if grep -r "sk-[a-zA-Z0-9]\{20,\}" --include="*.cpp" --include="*.h" --include="*.ui" . 2>/dev/null; then
    echo -e "${RED}✗ DANGER: Found potential API key in source code!${NC}"
    FOUND_ISSUES=$((FOUND_ISSUES + 1))
else
    echo -e "${GREEN}✓ No API keys found in source files${NC}"
fi
echo ""

# Check 2: Verify .gitignore exists and has proper entries
echo -e "${YELLOW}[2/5] Checking .gitignore configuration...${NC}"
if [ ! -f ".gitignore" ]; then
    echo -e "${RED}✗ WARNING: .gitignore file not found!${NC}"
    FOUND_ISSUES=$((FOUND_ISSUES + 1))
else
    if grep -q "\.conf" .gitignore && grep -q "\.db" .gitignore; then
        echo -e "${GREEN}✓ .gitignore properly configured${NC}"
    else
        echo -e "${RED}✗ WARNING: .gitignore missing critical entries${NC}"
        FOUND_ISSUES=$((FOUND_ISSUES + 1))
    fi
fi
echo ""

# Check 3: Verify no .conf or .db files are tracked by git
echo -e "${YELLOW}[3/5] Checking for tracked sensitive files...${NC}"
if git ls-files | grep -E "\.(conf|db|sqlite)$" > /dev/null 2>&1; then
    echo -e "${RED}✗ WARNING: Found tracked config/database files:${NC}"
    git ls-files | grep -E "\.(conf|db|sqlite)$"
    FOUND_ISSUES=$((FOUND_ISSUES + 1))
else
    echo -e "${GREEN}✓ No sensitive files tracked by git${NC}"
fi
echo ""

# Check 4: Check if API key is loaded from QSettings
echo -e "${YELLOW}[4/5] Verifying API key is loaded from QSettings...${NC}"
if grep -q "settings.value.*api_key" openai_client.cpp; then
    echo -e "${GREEN}✓ API key loaded from QSettings (secure)${NC}"
else
    echo -e "${RED}✗ WARNING: Cannot verify QSettings usage${NC}"
    FOUND_ISSUES=$((FOUND_ISSUES + 1))
fi
echo ""

# Check 5: Verify RPM spec doesn't package user data
echo -e "${YELLOW}[5/5] Checking RPM spec file...${NC}"
if [ -f "chatgpt-desktop.spec" ]; then
    if grep -q "%{_sysconfdir}\|%{_localstatedir}" chatgpt-desktop.spec; then
        echo -e "${RED}✗ WARNING: RPM spec may package config files${NC}"
        FOUND_ISSUES=$((FOUND_ISSUES + 1))
    else
        echo -e "${GREEN}✓ RPM spec looks safe${NC}"
    fi
else
    echo -e "${YELLOW}⚠ RPM spec file not found (skipping)${NC}"
fi
echo ""

# Summary
echo "======================================"
if [ $FOUND_ISSUES -eq 0 ]; then
    echo -e "${GREEN}✓ SECURITY CHECK PASSED${NC}"
    echo -e "${GREEN}Safe to commit and package!${NC}"
    exit 0
else
    echo -e "${RED}✗ SECURITY CHECK FAILED${NC}"
    echo -e "${RED}Found $FOUND_ISSUES issue(s) - DO NOT COMMIT!${NC}"
    exit 1
fi

