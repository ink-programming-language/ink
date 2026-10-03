// The entry imports a nested package module using its path-derived dotted identity.
from package.adapter import answer;

// The adapter's relative import must resolve to its package sibling.
func main(): i32
{
  return answer();
}
