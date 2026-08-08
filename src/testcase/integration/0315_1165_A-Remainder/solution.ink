// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var y: dynamic;
  var x: dynamic;
  var sum = 0;
  var str: dynamic;
  read(n, x, y, str);
  var len = (str.size() - 1);
  {
    var i = len;
    while ((i > (len - x)))
    {
      if ((str[i] == cpp_char("1")))
      {
        sum += 1;
      }
      i -= 1;
    }
  }
  if ((str[(len - y)] == cpp_char("0")))
  {
    sum += 1;
  } else
  {
    sum -= 1;
  }
  write(sum, "\n");
  return 0;
}
