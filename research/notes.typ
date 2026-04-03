#import "@preview/lovelace:0.3.0": *
#import "@preview/physica:0.9.8": *
#import "@preview/cetz:0.4.2": *

#set text(size: 7pt)

#show heading: set align(center)
#show heading: set text(weight: "bold")

#set page(margin: 0.8cm, paper: "us-letter")

#show heading: set align(center)
#show heading: set text(weight: "regular")

#let divider_rgb = rgb("#bcbcbc")
#let hl_bg = rgb("#bad5e3")

#let subdiv() = align(center)[
  #v(0.4em)
  #line(
    length: 100%,
    stroke: (paint: divider_rgb, thickness: 1pt, cap: "round",
    dash: "dashed")
  )
  #v(0.4em)
]

#let minidiv() = align(center)[
  #v(0.4em)
  #line(
    length: 40%,
    stroke: (paint: divider_rgb, thickness: 1pt, cap: "round",
    dash: "dash-dotted")
  )
  #v(0.4em)
]

#let div() = align(center)[
  #v(0.4em)
  #line(
    length: 100%,
    stroke: (paint: divider_rgb, thickness: 1pt, cap: "round")
  )
  #v(0.4em)
]

#let codeblock(language, body) = context[
  #set raw(
    theme: "assets/rose-pine-dawn.tmTheme",
  )
  #show raw: it => block(
    fill: rgb("#f6f2ee"),
    inset: 4pt,
    radius: 3pt,
    text(fill: rgb("#302b5d"), it)
  )
  #set text(size: 7.5pt)

  #raw(body, lang: language)
]

#let focus(content) = text(fill: blue)[
  #content
]

#let label(content) = text(font: "Libertinus Serif")[#content]

#let powerset = symbol(
  str.from-unicode(119979)
)

#let contradiction = symbol(
  str.from-unicode(8623)
)

#let slantf(num, denom) = math.frac(style: "skewed", num, denom)

#let circled(number, color) = box(
  width: 1em,
  height: 1em,
  box(
    baseline: 1.2em,
    stack(
      dir: btt,
      circle(width: 100%, fill: color),
      align(center)[
        #box(baseline: 0.8em, text(white, str(number)))
      ],
    ),
  ),
)

#link("https://doi.org/10.48550/arXiv.2208.13654")

#link("https://math.stanford.edu/~ryzhik/bubbles-final.pdf")

$
d P(t) eq - mu (1 - e^(P_0 - P(t))) dd(t) + sigma d B_t + nu S (P(t) - P(t - T)) dd(t)
$

- $mu$: regulate the strength of the mean revision
- $sigma$: regulate the strength of the random
- $nu$: regulate the strength of the speculative terms
  - assume $nu gt.double mu$
- $P_0$: fundamental price of the asset
- $B_t$: Brownian motion
- $S(x)$: Social response or speculative function
  - odd, monotone increasing
  - $S(x) arrow.r 1$ as $x arrow.r infinity$

$v S'(0) lt.double mu$

If both $nu$ and $sigma$ are small, the mean reversal term will always dominate

Large $sigma$ emphasizes randomness

Nonlinear response term $S(x) eq arctan(d x^(2 n + 1))$

Big rare bubbles phase, $nu gt.approx mu gt.approx sigma^2$

Strong randomness phase, $sigma^2 gt.approx nu gt.approx mu$

The balanced phase, $sigma tilde nu gt.approx mu$

#minidiv()

Price Formation:

$P_t eq 0.5 dot b_t^0 + 0.5 dot a_t^0$

$b_t^0$: best bid, $a_t^0$: best ask price

Fundamental trader:

$
D_(F T) eq kappa_1 (V_t - P_t) + kappa_2 (V_t - P_t)^3
$

Monument Trader: "Chartists", buy and sell financial assets after being influenced by recent price trends

Long-term Monument Trader: small $alpha$

Short-term Monument Trader: large $alpha$

Noise Trader: designed to capture other market activities that are not reflected by trend-following and value investing

Market Maker: creating realistic limit order book dynamics

