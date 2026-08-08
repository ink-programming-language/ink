// Translated from solution.cpp.

func err(it: dynamic)
{
  write("\n");
}

func err(it: dynamic, a: dynamic, args: dynamic...)
{
  write("[ ", (*it), " = ", a, " ] ");
  err(cpp_update(it, "++"), cpp_expand(args));
}

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var p: dynamic;
  scanf("%lld", (&(n)));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lld", (&(a[i])));
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      scanf("%lld", (&(b[i])));
      dif[i] = (a[i] - b[i]);
      i += 1;
    }
  }
  sort(dif.begin(), dif.end());
  var ans = 0;
  var maxi: dynamic;
  var r = dif.end();
  {
    i = 0;
    while ((i < (n - 1)))
    {
      maxi = (1 - dif[i]);
      var l = lower_bound(((dif.begin() + i) + 1), dif.end(), maxi);
      ans += ((r - l));
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
