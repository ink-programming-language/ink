// These imports reach the adapter and the storage provider through separate linked edges.
from adapter import update;
from memory import readValue;

// The target remains live in this frame after both imported callees return.
func main(): i32
{
  var Value: i32 = 0;
  var Returned = update(&Value);
  if (Value != 42 || *Returned != 42)
  {
    return 1;
  }
  *Returned = 41;
  if (readValue(&Value) != 41)
  {
    return 2;
  }
  return Value + 1;
}
