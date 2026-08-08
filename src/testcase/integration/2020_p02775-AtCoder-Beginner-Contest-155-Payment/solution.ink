// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var ans = 0;
  var moveUp = 0;
  {
    var i = (n.length() - 1);
    while ((i >= 0))
    {
      var num = ((cpp_cast(n.at(i)) - cpp_char("0")) + moveUp);
      if ((num < 5))
      {
        ans += num;
        moveUp = 0;
      } else if ((num == 5))
      {
        ans += num;
        moveUp = (if ((((cpp_cast(n[(i - 1)]) - cpp_char("0")) > 4))) 1 else 0);
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
