// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  var moveUp: dynamic = 0;
  {
    var i: dynamic = (n.length() - 1);
    while ((i >= 0))
    {
      var num: dynamic = ((cpp_cast(n.at(i)) - cpp_char("0")) + moveUp);
      if ((num < 5))
      {
        ans += num;
        moveUp = 0;
      } else if ((num == 5))
      {
        ans += num;
        moveUp = ( ((((cpp_cast(n[(i - 1)]) - cpp_char("0")) > 4))) ? 1 : 0);
      } else
      {
        ans += (10 - num);
        moveUp = 1;
      }
      i -= 1;
    }
  }
  write((ans + moveUp), "\n");
}
