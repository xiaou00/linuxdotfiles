function fish_greeting
    set -l theorem_names \
        "Yoneda's Lemma" \
        "Stokes's Theorem" \
        "First Isomorphism Theorem" \
        "Fermat's Last Theorem" \
        "Serre Duality"\
        "Kan Extension"\
        "Hahn-Banach Theorem"\
        "Parseval's Identity"\
        "Radon-Nikodym Theorem"\
        "Riemann Hypothesis"\
        "Quadratic Reciprocity Law"\
        "Spectral Sequence Convergence"\
        "Mordell-Weil Theorem"\
        "Grothendieck's Duality Theorem"\
        "Euler Characteristic"

    set -l theorem_formulas \
        "Hom(Hom(A,-),F) ≃ F(A)" \
        "∫_ᴍ dω = ∫_∂ᴍ ω" \
        "A/ker f ≃ im f ≃ coker f" \
        "xⁿ + y ⁿ ≠ zⁿ (n > 2) @ℤ" \
        "Hⁱ(X, F) ≃ Hⁿ⁻ⁱ(X, ω_X ⊗ Fᵛ)ᵛ"\
        "lim F ≃ RanₚF (p: J → 1)"\
        "∃f ∈ X* s.t. f|ₘ = φ and ‖f‖ = ‖φ‖"\
        "∑ |aₙ|² = 1/2π ∫ |f(x)|² dx"\
        "ν ≪ μ => dν = f dμ (f ∈ L¹)"\
        "ζ(s) = 0 => Re(s) = 1/2 (s ∉ -2ℕ)"\
        "(p/q)(q/p) = (-1)^((p-1)(q-1)/4)"\
        "E₂ᵖⁿ = Hᵖ(B; Hⁿ(F)) => Hᵖ⁺ⁿ(E)"\
        "E(ℚ) ≃ E_tor ⊕ ℤʳ"\
        "RHom(F, G) ≃ RHom(G, F ⊗ ω_X)ᵛ"\
        "χ = ∑ (-1)ⁱbᵢ = 2 - 2g"

    set -l count (count $theorem_names)
    set -l random_idx (random 1 $count)
    set -l today_theorem $theorem_names[$random_idx]
    set -l today_formula $theorem_formulas[$random_idx]
    echo (set_color white)"▄█████████▄       "(set_color normal)
    echo (set_color white)"█████████▓▓       "(set_color cyan)$today_theorem(set_color normal)
    echo (set_color white)"█████████▓▓       "$today_formula(set_color normal)
    echo (set_color white)"█▄▄"(set_color red)"▄"(set_color white)"▄▄▄▄▄█████▀  "(set_color red)" Daily Math"(set_color normal)
    echo (set_color white)"▀██"(set_color red)"█"(set_color white)"████████▀    "(set_color normal)
end
