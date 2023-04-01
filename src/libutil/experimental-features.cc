#include "experimental-features.hh"
#include "util.hh"

#include "nlohmann/json.hpp"

namespace nix {

static const RegisterExperimentalFeature xpCaDerivations({
    .experimentalFeature = Xp::CaDerivations,
    .featureName = "ca-derivations",
    .featureDoc = R"(
      Allows derivations to be content-addressed in order to prevent rebuilds
      when changes to the derivation do not result in changes to the
      derivation's output. See
      [__contentAddressed](../language/advanced-attributes.md#adv-attr-__contentAddressed)
      for more info.
    )"
});

static const RegisterExperimentalFeature xpImpureDerivations({
    .experimentalFeature = Xp::ImpureDerivations,
    .featureName = "impure-derivations",
    .featureDoc = R"(
      Allows derivations to produce non-fixed outputs by setting the `__impure`
      derivation attribute to `true`. See [these release
      notes](../release-notes/rl-2.8.md) for an example.
    )"
});

static const RegisterExperimentalFeature xpFlakes({
    .experimentalFeature = Xp::Flakes,
    .featureName = "flakes",
    .featureDoc = R"(
      Allows for derivations to be packaged in flakes. See the manual entry for
      [`nix flake`](../command-ref/new-cli/nix3-flake.md) or this [detailed
      introduction](https://www.tweag.io/blog/2020-05-25-flakes/) for more info.
    )"
});

static const RegisterExperimentalFeature xpNixCommand({
    .experimentalFeature = Xp::NixCommand,
    .featureName = "nix-command",
    .featureDoc = R"(
      Allows the usage of the new `nix` CLI subcommands, such as `nix build`, `nix
      develop`, `nix run`, etc. See the manual for
      [`nix`](../command-ref/new-cli/nix.md) for more info.
    )"
});

static const RegisterExperimentalFeature xpRecursiveNix({
    .experimentalFeature = Xp::RecursiveNix,
    .featureName = "recursive-nix",
    .featureDoc = R"(
      Allow Nix derivations to call Nix in order to recursively build derivations.
      See [this
      commit](https://github.com/edolstra/nix/commit/1a27aa7d64ffe6fc36cfca4d82bdf51c4d8cf717)
      for more info.
    )"
});

static const RegisterExperimentalFeature xpNoUrlLiterals({
    .experimentalFeature = Xp::NoUrlLiterals,
    .featureName = "no-url-literals",
    .featureDoc = R"(
      Disallows unquoted URLs as part of the Nix language syntax. See [RFC
      45](https://github.com/NixOS/rfcs/pull/45) for more info.
    )"
});

static const RegisterExperimentalFeature xpFetchClosure({
    .experimentalFeature = Xp::FetchClosure,
    .featureName = "fetch-closure",
    .featureDoc = R"(
      Enables the use of the `fetchClosure` function in the standard library. See
      the docs for [`fetchClosure`](../language/builtins.md#builtins-fetchClosure)
      for more info.
    )"
});

static const RegisterExperimentalFeature xpReplFlake({
    .experimentalFeature = Xp::ReplFlake,
    .featureName = "repl-flake",
    .featureDoc = R"(
      Allows the user to enter a Nix REPL within a flake, e.g. `nix repl nixpkgs`
      or `nix repl .#foo`.
    )"
});

static const RegisterExperimentalFeature xpAutoAllocateUids({
    .experimentalFeature = Xp::AutoAllocateUids,
    .featureName = "auto-allocate-uids",
    .featureDoc = R"(
      Allows Nix to automatically pick UIDs for builds, rather than creating
      `nixbld*` user accounts. See [here](#conf-auto-allocate-uids) for more info.
    )"
});

static const RegisterExperimentalFeature xpCgroups({
    .experimentalFeature = Xp::Cgroups,
    .featureName = "cgroups",
    .featureDoc = R"(
      Allows Nix to execute builds inside cgroups. See
      [`use-cgroups`](#conf-use-cgroups) for more info.
    )"
});

static const RegisterExperimentalFeature xpDiscardReferences({
    .experimentalFeature = Xp::DiscardReferences,
    .featureName = "discard-references",
    .featureDoc = R"(
      Enables the use of the `unsafeDiscardReferences` attribute in derivations
      that use structured attributes. This disables scanning of outputs for
      runtime dependencies.
    )"
});

RegisterExperimentalFeature::RegisterExperimentalFeature(RegisterExperimentalFeature::FullExperimentalFeature&& feature)
{
    fullXpFeatures.push_back(std::move(feature));
}

const std::optional<ExperimentalFeature> parseExperimentalFeature(const std::string_view & name)
{
    using ReverseXpMap = std::map<std::string_view, ExperimentalFeature>;

    static auto reverseXpMap = []()
    {
        auto reverseXpMap = std::make_unique<ReverseXpMap>();
        for (auto & [feature, name] : stringifiedXpFeatures) {
            (*reverseXpMap)[name] = feature;
        }
        return reverseXpMap;
    }();

    if (auto feature = get(*reverseXpMap, name))
        return *feature;
    else
        return std::nullopt;
}

std::string_view showExperimentalFeature(const ExperimentalFeature feature)
{
    const auto ret = get(stringifiedXpFeatures, feature);
    assert(ret);
    return *ret;
}


std::set<ExperimentalFeature> parseFeatures(const std::set<std::string> & rawFeatures)
{
    std::set<ExperimentalFeature> res;
    for (auto & rawFeature : rawFeatures) {
        if (auto feature = parseExperimentalFeature(rawFeature))
            res.insert(*feature);
    }
    return res;
}

MissingExperimentalFeature::MissingExperimentalFeature(ExperimentalFeature feature)
    : Error("experimental Nix feature '%1%' is disabled; use '--extra-experimental-features %1%' to override", showExperimentalFeature(feature))
    , missingFeature(feature)
{}

std::ostream & operator <<(std::ostream & str, const ExperimentalFeature & feature)
{
    return str << showExperimentalFeature(feature);
}

void to_json(nlohmann::json & j, const ExperimentalFeature & feature)
{
    j = showExperimentalFeature(feature);
}

void from_json(const nlohmann::json & j, ExperimentalFeature & feature)
{
    const std::string input = j;
    const auto parsed = parseExperimentalFeature(input);

    if (parsed.has_value())
        feature = *parsed;
    else
        throw Error("Unknown experimental feature '%s' in JSON input", input);
}

}
