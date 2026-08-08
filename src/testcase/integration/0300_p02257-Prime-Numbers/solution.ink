// Translated from solution.cpp.

func prime(p: dynamic)
{
  {
    var i = 2;
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

func main()
{
  var n: dynamic;
  var p: dynamic;
  var ans = 0;
  read(n);
  {
    var i = 0;
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
