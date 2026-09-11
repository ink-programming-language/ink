// Translated from solution.cpp.

func err(it: dynamic) -> dynamic
{
  write("\n");
}

func err(it: dynamic, a: dynamic, args: dynamic...) -> dynamic
{
  write("[ ", (*it), " = ", a, " ] ");
  err(cpp_update(it, "++"), cpp_expand(args));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
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
  var ans: dynamic = 0;
  var maxi: dynamic = cpp_uninitialized();
  var r: dynamic = dif.end();
  {
    i = 0;
    while ((i < (n - 1)))
    {
      maxi = (1 - dif[i]);
      var l: dynamic = lower_bound(((dif.begin() + i) + 1), dif.end(), maxi);
      ans += ((r - l));
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
