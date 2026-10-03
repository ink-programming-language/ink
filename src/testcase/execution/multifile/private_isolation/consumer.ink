// Public wrappers can be imported without making their private helpers visible.
from left_provider import leftValue as left;
from right_provider import rightValue as right;

func main(): i32
{
  return left() + right();
}
