// Local aliases import two answer functions while preserving each declaration's qualified module identity.
from package.left import answer as leftAnswer;
from package.right import answer as rightAnswer;

// Both symbols must resolve to their own modules regardless of input object order.
func main(): i32
{
  return leftAnswer() + rightAnswer();
}
