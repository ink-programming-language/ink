// The alias retains the actual signature and identity imported from leaf.ink.
from leaf import answer as upstream;

// The middle module executes its imported callee before adding its own contribution.
func answer(): i32
{
  return upstream() + 20;
}
