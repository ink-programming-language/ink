// Translated from solution.cpp.

func lucky(i: dynamic) -> dynamic
{
  var val: dynamic = to_string(i);
  for (var i: dynamic in val)
  {
    if ((i == cpp_char("8")))
    {
      return true;
    }
  }
  return false;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var cnt: dynamic = 1;
  {
    var i: dynamic = (n + 1);
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
