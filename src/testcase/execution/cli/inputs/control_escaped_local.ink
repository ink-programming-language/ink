// A branch-local declaration must not be visible after the branch has ended.
func main(): i32
{
  if (true)
  {
    var Inner: i32 = 7;
  }
  return Inner;
}
