// Translated from solution.cpp.

func prime(p: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while (((i * i) <= p))
    {
      if ((!((p % i))))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(p);
      if (prime(p))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
