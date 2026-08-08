// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var i: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  var ans = 0;
  {
    i = (n - 1);
    while ((i >= 0))
    {
      a[i] += ans;
      if (((a[i] % b[i]) != 0))
      {
        ans += (b[i] - (a[i] % b[i]));
      }
      i -= 1;
    }
  }
  write(ans);
}
