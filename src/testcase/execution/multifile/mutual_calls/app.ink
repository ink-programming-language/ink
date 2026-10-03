// Import both ends of the cycle so each can serve as the first activation.
from first import countFirst;
from second import countSecond;

// Both direct entry edges and repeated first-to-second-to-first calls contribute to the result.
func main(): i32
{
  return countFirst(20) + countSecond(22);
}
