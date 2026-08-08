// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(val[i]);
      i += 1;
    }
  }
  sort(val.begin(), val.end());
  if ((n == 1))
  {
    write(val[0]);
    return 0;
  }
  var ans = val[0];
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      if ((val[i] >= 0))
      {
        ans -= val[i];
      }
      i += 1;
    }
  }
  ans = (val.back() - ans);
  {
    var i = 1;
    while ((i < (n - 1)))
    {
      if ((val[i] < 0))
      {
        ans -= val[i];
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
