// Translated from solution.cpp.

func lucky(i: dynamic)
{
  var val = to_string(i);
  for (var i in val)
  {
    if ((i == cpp_char("8")))
    {
      return true;
    }
  }
  return false;
}

func main()
{
  var n: dynamic;
  read(n);
  var cnt = 1;
  {
    var i = (n + 1);
    while (true)
    {
      if (lucky(i))
      {
        break;
      }
      cnt += 1;
      i += 1;
    }
  }
  write(cnt, "\n");
}
