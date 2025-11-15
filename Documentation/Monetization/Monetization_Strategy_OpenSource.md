# ChromaRunner - Open Source Monetization Strategy

**Document Version**: 2.0 (REVISED for Open Source)
**Date**: 2025-11-15
**Status**: Open Source Edition
**Critical Context**: **Source code is open source on GitHub**

---

## Executive Summary

### The Open Source Reality

**Fundamental Fact**: ChromaRunner's source code is **publicly available on GitHub** under an open-source license. This changes everything about monetization strategy.

**Recommended Model**: **Hybrid Open Source + Paid Convenience**

### Final Recommendation

**Business Model**: Dual-licensing with ethical "convenience pricing"
- **GitHub**: Free source code (GPL/MIT license) - compile yourself
- **Steam**: $4.99 compiled binaries with Steam features (auto-updates, achievements, cloud saves, Workshop support)
- **itch.io**: Pay-What-You-Want ($0-$10 suggested $4.99)
- **Philosophy**: "Pay for convenience and support, not for code access"

**Precedent**: This model works successfully:
- **Mindustry**: Open source, sells 500K+ copies on Steam at $6.99
- **Barotrauma**: Open source, sells 1M+ copies on Steam at $29.99
- **Cataclysm: Dark Days Ahead**: Free on GitHub, PWYW on itch.io

### Revenue Projection (Revised for Open Source)

| Scenario | Steam Sales | Avg Price | itch.io PWYW | GitHub Sponsors | Net Revenue |
|----------|-------------|-----------|--------------|-----------------|-------------|
| **Conservative** | 1,500 | $4.25 | $500 | $200 | **$6,200** |
| **Moderate** | 5,000 | $4.50 | $2,000 | $1,000 | **$21,750** |
| **Optimistic** | 15,000 | $4.75 | $5,000 | $3,000 | **$68,625** |

**Key Difference from Closed-Source**: Expect 30-50% lower sales due to free GitHub alternative, but higher community goodwill and contributions.

---

## Why Open Source + Paid Works

### The Value Proposition for Buyers

**What Steam/itch.io buyers get that GitHub doesn't:**

✅ **Convenience**:
- One-click install (no compilation, no dependencies, no build errors)
- Auto-updates (no git pull, no rebuild, always latest)
- Uninstall via Steam (clean removal)

✅ **Steam Platform Features**:
- **Achievements**: Full unlock system (not available in source builds)
- **Cloud Saves**: Continue across devices
- **Steam Leaderboards**: Global competition
- **Workshop Support**: Easy mod installation
- **Overlay**: Screenshots, FPS counter, friends list
- **Proton Compatibility**: Linux gaming verified

✅ **Support the Developer**:
- Ethical "tip jar" for enjoying the game
- Fund future development
- Enable full-time indie development

✅ **Quality Assurance**:
- Tested builds (no experimental Git commits)
- Stable releases (tagged versions)
- Official support (bug reports prioritized)

### The Value Proposition for GitHub Users

**What's free on GitHub:**
- ✅ Full source code access
- ✅ Learning resource (study Unreal Engine C++ patterns)
- ✅ Modding foundation (create custom content)
- ✅ Fork and customize (make it yours)
- ✅ Educational use (schools, tutorials)
- ✅ Contribution opportunities (pull requests welcome)

**Trade-offs:**
- ⚠️ Must compile from source (Unreal Engine 5.5 required)
- ⚠️ No Steam achievements/leaderboards
- ⚠️ Manual updates (git pull + rebuild)
- ⚠️ Experimental builds (main branch may be unstable)

---

## Ethical Communication Strategy

### Store Page Messaging (Critical for Trust)

**Steam Store Page** (above the fold):
```
ChromaRunner is an open-source 2.5D platformer built with Unreal Engine 5.5.

💰 This purchase supports the developer and provides:
• One-click installation (no compilation required)
• Auto-updates and cloud saves
• Steam achievements and global leaderboards
• Workshop mod support

📂 Source code is freely available on GitHub:
  github.com/[YOUR_GITHUB_USERNAME]/ChromaRunner

This is "convenience pricing" - you're supporting indie development
while getting the best experience. Thank you for choosing to support!
```

**itch.io Page**:
```
Pay-What-You-Want ($0-$10, suggested $4.99)

This game is open source on GitHub. You can compile it for free.
If you choose to pay, you're directly supporting the developer.

Every dollar helps fund:
- New content updates
- Bug fixes and optimizations
- Full-time indie development
- Future game projects

Downloads include compiled binaries for Windows/Linux/Mac.
```

### Reddit/Social Media Messaging

**When announcing:**
```
"ChromaRunner is now on Steam! 🎮

📂 Open Source: github.com/[username]/ChromaRunner
💰 Steam: $4.99 for compiled binaries + Steam features
🎁 itch.io: Pay-What-You-Want (including $0)

You can compile from source for free, or support development
via Steam/itch.io for convenience. Your choice, always.

Thanks to everyone who contributed and tested! 🙏"
```

**Key Principles**:
- ✅ Always mention open-source status prominently
- ✅ Make GitHub link highly visible
- ✅ Frame payment as "support" not "purchase"
- ✅ Emphasize Steam features as value-add
- ❌ Never hide the fact it's open source
- ❌ Don't make users feel tricked after buying

---

## Revenue Model Breakdown

### Primary Revenue Stream: Steam Sales

**Why Steam works for open source:**
- Most gamers don't know how to compile C++ Unreal projects
- Steam features (achievements, leaderboards, Workshop) have real value
- Social proof (reviews, playtime) builds trust
- Discoverability (tags, recommendations) reaches non-GitHub audience

**Expected Conversion**:
- **GitHub visitors who buy Steam**: 5-10%
- **Steam-only buyers (unaware of GitHub)**: 85-90%
- **Steam buyers who later find GitHub**: 5% (minimal refunds if messaging is clear)

**Pricing**: $4.99 (same as closed-source)
- Low enough to be impulse purchase
- High enough to signal quality
- Competitive with similar indie platformers

### Secondary Revenue: itch.io PWYW

**Pay-What-You-Want Advantages**:
- Captures price-sensitive users ($0-$2 buyers)
- Enables generous supporters ($10+ tips)
- DRM-free positioning (appeals to FOSS community)
- Lower barrier to try (risk-free)

**Expected Average**: $2.50 per download
- 40% pay $0 (free)
- 30% pay $1-$2
- 20% pay $4.99 (suggested)
- 10% pay $5-$10 (supporters)

**Revenue Projection**: $500-$5,000 depending on traffic

### Tertiary Revenue: GitHub Sponsors / Patreon

**Monthly Supporter Tiers**:
- **$1/mo**: Support the developer, name in credits
- **$5/mo**: Early access to experimental features
- **$10/mo**: Vote on next features, Discord role
- **$25/mo**: Direct feedback sessions, design input

**Expected Backers** (if actively promoted):
- Conservative: 20 backers avg $5/mo = **$1,200/year**
- Moderate: 100 backers avg $10/mo = **$12,000/year**
- Optimistic: 300 backers avg $10/mo = **$36,000/year**

**Key**: This is recurring revenue (more sustainable than one-time sales)

### Quaternary Revenue: Asset Sales

**What to sell separately:**
- **Soundtrack**: $1.99 (Bandcamp, Steam DLC)
- **Art Pack**: $4.99 (all sprites, backgrounds PSD files for modders)
- **Level Editor Guide**: $2.99 (tutorial for making custom levels)

**Revenue Potential**: $1,000-$10,000 (Year 1)

---

## Licensing Strategy

### Recommended License: GPL v3 or MIT

**GPL v3** (Copyleft):
- ✅ Protects your code from becoming proprietary
- ✅ Forces derivatives to stay open source
- ✅ Prevents commercial exploitation without contribution
- ❌ More restrictive (some devs avoid GPL)

**MIT License** (Permissive):
- ✅ Maximum freedom for users
- ✅ Allows commercial forks (builds community)
- ✅ Simple, widely understood
- ❌ No protection against proprietary forks

**Recommendation for ChromaRunner**: **MIT License**
- Aligns with "educational resource" positioning
- Encourages modding and learning
- Builds maximum goodwill
- Allows Unreal Marketplace asset integration

### Dual-Licensing (Advanced)

**Option**: License code as GPL, but sell commercial exception licenses

Example:
- Free under GPL (must share modifications)
- $499 commercial license (use in closed-source games)

**Benefit**: Captures revenue from studios using your code
**Complexity**: Requires legal clarity, enforcement

**Recommendation**: Start with simple MIT, consider dual-licensing Year 2

---

## Marketing Strategy for Open Source

### Unique Advantages

**Open Source as Marketing Tool**:
1. **GitHub Stars**: Trending on GitHub = free visibility
2. **Developer Community**: Gamedev students study your code
3. **Media Coverage**: "Open source indie game" is newsworthy
4. **Contributor Network**: Community fixes bugs, adds features
5. **Educational Use**: Schools/bootcamps feature your project
6. **Portfolio Boost**: "Shipped open-source UE5 game" = hiring signal

### Marketing Channels (Prioritized)

**1. GitHub Community** (Highest ROI)
- [ ] README with compelling GIFs, clear build instructions
- [ ] "Good First Issue" labels for new contributors
- [ ] CONTRIBUTING.md with code standards
- [ ] Regular releases with changelogs
- [ ] GitHub Discussions for community Q&A
- [ ] Sponsor button prominently displayed

**2. Open Source Communities**
- [ ] Post to r/opensource, r/linux_gaming, r/godot (Unreal too)
- [ ] Hacker News "Show HN" post
- [ ] Open Source Game Dev Discord servers
- [ ] Libre Game Night showcases
- [ ] FOSDEM (Free and Open Source Software Developers' European Meeting)

**3. Gamedev Communities** (Same as closed-source)
- [ ] r/gamedev, r/IndieGaming, r/UnrealEngine
- [ ] Twitter #indiedev, #gamedev, #madewithunreal
- [ ] TikTok/YouTube Shorts (gameplay clips)

**4. Steam Community** (Target non-technical gamers)
- [ ] Position as "premium experience of open-source game"
- [ ] Emphasize convenience, not code access
- [ ] Highlight Steam features

### Community Management

**GitHub Issues = Customer Support**:
- Respond to bug reports within 24 hours
- Label issues clearly (bug, enhancement, question)
- Close stale issues (30 days no activity)
- Feature active contributors in release notes

**Pull Request Policy**:
- Welcome all contributions (code, docs, art)
- Provide constructive feedback
- Merge quality PRs quickly (reward effort)
- Feature top contributors in credits

---

## Revenue Projections (18 Months)

### Conservative Scenario

| Source | Revenue |
|--------|---------|
| Steam (1,500 sales @ $4.25 avg) | $4,460 net |
| itch.io (500 downloads @ $1 avg) | $450 net |
| GitHub Sponsors (20 @ $5/mo × 12) | $1,200 |
| Soundtrack DLC | $200 |
| **Total** | **$6,310** |

**Breakeven**: 500 Steam sales ($1,750 net) covers $1,000 dev costs

### Moderate Scenario

| Source | Revenue |
|--------|---------|
| Steam (5,000 sales @ $4.50 avg) | $15,750 net |
| itch.io (2,000 downloads @ $2 avg) | $2,800 net |
| GitHub Sponsors (100 @ $10/mo × 12) | $12,000 |
| Cosmetic DLC Pack | $2,000 |
| Soundtrack | $500 |
| **Total** | **$33,050** |

### Optimistic Scenario

| Source | Revenue |
|--------|---------|
| Steam (15,000 sales @ $4.75 avg) | $49,875 net |
| itch.io (5,000 downloads @ $3 avg) | $10,500 net |
| GitHub Sponsors (300 @ $10/mo × 18) | $54,000 |
| Content Expansion DLC | $8,000 |
| Art Asset Pack | $3,000 |
| Soundtrack | $1,500 |
| **Total** | **$126,875** |

**Key Insight**: Open source unlocks **recurring revenue via Sponsors** which closed-source doesn't have. This can exceed one-time sales over time.

---

## Risk Analysis (Open Source Specific)

### Risk 1: "Why buy if it's free?"

**Likelihood**: High (50% of Steam page visitors will check GitHub)
**Impact**: Medium (reduces sales by 30-50% vs closed-source)

**Mitigation**:
1. Clear messaging on Steam page (transparent from the start)
2. Emphasize Steam features as value-add (achievements, cloud saves)
3. Target non-technical audience (casual gamers who won't compile)
4. Offer itch.io PWYW for price-sensitive users
5. Make compilation genuinely difficult (Unreal Engine 5.5 setup is complex)

**Expected Outcome**: 60-70% of potential buyers still purchase for convenience

### Risk 2: Commercial Forks (Someone Resells Your Game)

**Likelihood**: Low (requires effort, damages reputation)
**Impact**: Medium (dilutes your sales)

**Mitigation**:
1. MIT license allows this (it's legal) - can't prevent
2. Brand protection: Your version is "official", forks are unofficial
3. Community support: Real community rallies against scammers
4. Steam/itch.io reporting: Report unauthorized clones
5. Better marketing: Out-compete lazy clones with quality and updates

**Expected Outcome**: Rare occurrence, minimal revenue impact (<5%)

### Risk 3: Contributor Drama

**Likelihood**: Low (small project)
**Impact**: High (public arguments damage reputation)

**Mitigation**:
1. Clear Code of Conduct (be respectful or be banned)
2. Benevolent dictator model (you have final say)
3. Contribution guidelines (set expectations upfront)
4. Private communication for conflicts (don't argue in issues)
5. Credit all contributors (reduce resentment)

**Expected Outcome**: Healthy community with occasional moderation needed

### Risk 4: Low GitHub Stars (No Viral Growth)

**Likelihood**: Medium
**Impact**: Medium (miss free marketing opportunity)

**Mitigation**:
1. Quality README (GIFs, clear value proposition, build instructions)
2. Post to r/opensource, Hacker News at launch
3. "Open source game" angle in all PR
4. Tag releases properly (semantic versioning)
5. Engage with gamedev learning community

**Expected Outcome**: 500-2,000 stars (Year 1), moderate visibility

---

## Release Roadmap (Revised for Open Source)

### Phase 1: Pre-Launch (Months 1-3)

**Month 1: GitHub Foundation**
- [ ] Clean up repository structure
- [ ] Write comprehensive README with build instructions
- [ ] Add LICENSE file (MIT recommended)
- [ ] Create CONTRIBUTING.md
- [ ] Add GitHub Sponsors button
- [ ] Tag initial release (v0.5-alpha)
- [ ] Post to r/opensource: "Open source 2.5D platformer in UE5"

**Month 2: Community Building**
- [ ] Create Steam store page (mention open source in description)
- [ ] Set up itch.io PWYW page (link to GitHub)
- [ ] Weekly devlogs (Reddit, GitHub Discussions, Twitter)
- [ ] "Good First Issue" labels for new contributors
- [ ] Engage with open-source gamedev Discord servers

**Month 3: Pre-Launch Push**
- [ ] GitHub Sponsors tiers configured
- [ ] Patreon alternative (if Sponsors doesn't work)
- [ ] Demo builds for content creators (Steam + standalone)
- [ ] Press kit emphasizing "open source indie game" angle
- [ ] Hacker News "Show HN" post (timing: Tuesday 9 AM PST)

**Goal**: 500+ GitHub stars, 1,000+ Steam wishlists, 50+ Sponsors

---

### Phase 2: Early Access Launch (Month 4)

**Launch Day Strategy**

**Simultaneous Launches**:
- [ ] Steam Early Access: $3.99 (20% off)
- [ ] itch.io PWYW: $0-$10 (suggested $4.99)
- [ ] GitHub Release: v0.8-early-access (stable tag)

**Marketing Blitz**:
- [ ] Reddit posts: r/opensource, r/linux_gaming, r/IndieGaming, r/UnrealEngine
- [ ] Hacker News: "We built an open-source platformer with Unreal Engine 5"
- [ ] Twitter thread: "After 12 months, ChromaRunner is live - free on GitHub, $4 on Steam"
- [ ] Email Steam wishlist (auto-notify)
- [ ] Email GitHub Sponsors (early access perk)

**Post-Launch (Week 1-4)**:
- [ ] Daily GitHub issue triage
- [ ] Respond to all Steam reviews (thank supporters)
- [ ] Hotfix critical bugs within 48 hours
- [ ] Weekly devlog (progress + contributor highlights)
- [ ] Feature top contributors in release notes

**Success Metrics**:
- ✅ 250+ Steam sales (Week 1)
- ✅ 1,000+ itch.io downloads
- ✅ 100+ GitHub stars (launch spike)
- ✅ 75%+ positive Steam reviews
- ✅ 10+ contributors (PRs merged)

---

### Phase 3: Full Launch (Month 7)

**What Changes**:
- Early Access tag removed
- Price increase: $3.99 → $4.99
- Content complete (5+ biomes, 50+ obstacles)
- Achievements unlocked
- Leaderboards live
- Workshop support enabled

**Marketing Push #2**:
- [ ] Trailer v2 (emphasize open source + Steam features)
- [ ] Press outreach: "Open source game reaches 1.0"
- [ ] GitHub trending push (ask community to star)
- [ ] Reddit AMA on r/opensource + r/IndieGaming

---

### Phase 4: Long-Term Sustainability (Months 8-18)

**Content Updates** (All free):
- Month 10: New biome (Underwater)
- Month 12: Challenge mode (Daily runs)
- Month 15: Level editor (Steam Workshop)

**Paid DLC** (Optional support):
- Month 12: Cosmetic pack ($1.99) - character skins
- Month 15: Soundtrack ($1.99) - support the composer

**Community Growth**:
- GitHub Sponsors: Target 100+ backers ($10/mo avg)
- Contributor onboarding: 50+ merged PRs
- Modding community: 20+ Workshop items

---

## Platform Strategy (Open Source Edition)

### GitHub (Primary Platform for Code)

**Why GitHub First**:
- Free hosting and distribution
- Built-in community tools (Issues, Discussions, Actions)
- Discoverability (trending, topics, search)
- Contributor pipeline (PRs, forks)
- No platform fees (100% tips via Sponsors)

**Optimization**:
- [ ] Compelling README with GIFs
- [ ] CI/CD pipeline (GitHub Actions auto-build)
- [ ] Release binaries attached to tags
- [ ] GitHub Pages for documentation
- [ ] Topics: `unreal-engine`, `platformer`, `2d-game`, `endless-runner`

### Steam (Primary Platform for Sales)

**Why Steam Still Works**:
- Casual gamers don't visit GitHub
- Steam features add genuine value
- Social proof (reviews) builds trust
- Wishlist system drives launch velocity

**Positioning**: "Official Steam version with achievements, cloud saves, and auto-updates"

### itch.io (Secondary Platform)

**Why itch.io for Open Source**:
- PWYW pricing aligns with FOSS philosophy
- 10% platform fee (vs Steam's 30%)
- DRM-free positioning
- Indie-friendly community

**Positioning**: "Support the dev edition - same game, direct support"

---

## Ethical Monetization (Open Source Edition)

### What's Always Free

**Core Game** (GitHub):
- ✅ Full source code
- ✅ Compile your own builds
- ✅ All gameplay content
- ✅ Modding capabilities
- ✅ Educational use

**No Paywalls**:
- ❌ No "premium source code"
- ❌ No "pro features" in code
- ❌ No time-limited "trial" builds
- ❌ No dark patterns to force payment

### What You Can Ethically Sell

**Steam Version**:
- ✅ Compiled binaries (convenience)
- ✅ Steam platform features (achievements, cloud, Workshop)
- ✅ Official support (bug priority)
- ✅ Auto-updates (no manual git pull)

**Cosmetic DLC**:
- ✅ Art packs (supporter content)
- ✅ Soundtrack (music licensing separate from code)
- ✅ "Thank you" DLC (0 content, 100% tip)

**Services**:
- ✅ Custom development (paid feature requests)
- ✅ Consulting (help others build with your code)
- ✅ Tutorials (video courses on Unreal Engine)

---

## Competitive Advantage: Open Source

### Why Open Source Helps Marketing

1. **Media Angle**: "Open source UE5 platformer" = unique story
2. **Educational Market**: Schools/bootcamps feature your project
3. **Developer Audience**: Gamedevs buy to study your code
4. **Community Contributions**: Free QA, bug fixes, content
5. **Goodwill**: FOSS community actively promotes ethical projects
6. **Longevity**: Game lives beyond you (community can fork if abandoned)
7. **Portfolio**: "Shipped commercial open-source game" = hiring signal
8. **Cross-Promotion**: Other FOSS games link to yours

### Successful Open Source Game Examples

| Game | Model | Result |
|------|-------|--------|
| **Mindustry** | MIT license, $6.99 Steam | 500K+ sales, thriving mods |
| **Barotrauma** | GPL, $29.99 Steam | 1M+ sales, active contributors |
| **Endless Sky** | GPL, PWYW | 50K+ downloads, community-driven |
| **0 A.D.** | GPL, $0 donations | $100K+ donations over 10 years |

**Key Lesson**: Open source + Steam works if:
1. Compilation is genuinely difficult (Unreal Engine is perfect for this)
2. Steam features add real value (achievements, Workshop)
3. Messaging is transparent ("support the dev")
4. Community is cultivated (respond to PRs, engage contributors)

---

## Action Plan (Next 30 Days)

### Week 1: Repository Cleanup
- [ ] Review all code for sensitive data (API keys, credentials)
- [ ] Add MIT LICENSE file
- [ ] Write comprehensive README (build instructions, screenshots, features)
- [ ] Create CONTRIBUTING.md with code style guide
- [ ] Add CODE_OF_CONDUCT.md (use Contributor Covenant)
- [ ] Tag current version (v0.7-beta)

### Week 2: GitHub Optimization
- [ ] Enable GitHub Discussions
- [ ] Create GitHub Sponsors profile
- [ ] Set sponsor tiers ($1, $5, $10, $25/mo)
- [ ] Add "Good First Issue" labels (10+ issues)
- [ ] Set up GitHub Actions (auto-build on push)
- [ ] Create GitHub Pages site with documentation

### Week 3: Steam/itch.io Setup
- [ ] Create Steamworks account ($100 fee)
- [ ] Draft Steam store page
  - Mention open source in first paragraph
  - Link to GitHub repository
  - Emphasize Steam features
- [ ] Create itch.io page (PWYW $0-$10)
- [ ] Link all platforms to each other

### Week 4: Marketing Launch
- [ ] Post to r/opensource: "We built an open-source platformer in UE5"
- [ ] Post to r/unrealengine: "ChromaRunner - open source 2.5D runner"
- [ ] Hacker News "Show HN" (Tuesday 9 AM PST)
- [ ] Twitter thread announcing open source availability
- [ ] First devlog on GitHub Discussions

---

## Success Metrics (18-Month Goals)

### GitHub Metrics
- ✅ 1,000+ stars
- ✅ 50+ contributors
- ✅ 200+ forks
- ✅ 500+ closed issues
- ✅ 100+ merged PRs

### Revenue Metrics
- ✅ $20,000+ net revenue (Year 1)
- ✅ 100+ active GitHub Sponsors ($10/mo avg)
- ✅ 5,000+ Steam sales
- ✅ 10,000+ itch.io downloads

### Community Metrics
- ✅ 20+ Workshop mods
- ✅ 10+ YouTube tutorials using your code
- ✅ 5+ schools/bootcamps featuring project
- ✅ 100+ Discord community members

---

## Conclusion

**ChromaRunner as an open-source indie game has a unique opportunity:**

✅ **Financial Sustainability**: Dual-licensing with Steam convenience pricing can generate $20K-$50K while staying ethically open
✅ **Community Power**: Contributors fix bugs, add features, and promote your game for free
✅ **Educational Impact**: Thousands of devs learn Unreal Engine from your code
✅ **Longevity**: Game outlives you through community ownership
✅ **Marketing Advantage**: "Open source" is a newsworthy story in indie gaming
✅ **Recurring Revenue**: GitHub Sponsors provides stable monthly income

**Critical Success Factors**:
1. **Transparency**: Always mention open source on Steam/itch.io (build trust)
2. **Steam Value**: Achievements, Workshop, cloud saves justify $4.99 convenience price
3. **Community Care**: Respond to PRs, merge contributions, feature top contributors
4. **Quality Documentation**: README is your first impression (GIFs, clear builds, quick start)
5. **Consistent Updates**: Bi-weekly releases maintain GitHub trending visibility

**The Mindustry Model works**: 500K+ Steam sales of an open-source game proves it's viable. Focus on quality, transparency, and community.

---

**Next Steps**:
1. ✅ Read and approve this revised open-source strategy
2. [ ] Clean up repository and add LICENSE (Week 1)
3. [ ] Set up GitHub Sponsors + Steam page (Week 2-3)
4. [ ] Launch on r/opensource and Hacker News (Week 4)
5. [ ] Begin 18-month execution roadmap

**Remember**: Open source is not a compromise—it's a competitive advantage. Execute with confidence.

---

**Document Maintained By**: ChromaRunner Development Team
**Last Updated**: 2025-11-15
**Status**: Active Strategy - Open Source Edition
**Replaces**: Monetization_Strategy.md v1.0 (closed-source assumptions)
