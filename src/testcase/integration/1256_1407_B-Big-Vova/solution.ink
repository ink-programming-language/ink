// Translated from solution.cpp.

func fastPow(a: dynamic, p: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (p)
  {
    if ((p & 1))
    {
      res = (((res * a)) % 1000000007);
    }
    p >>= 1;
    a = (((a * a)) % 1000000007);
  }
  return res;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func solve(num: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var ans: dynamic = cpp_uninitialized();
  var used: dynamic = cpp_construct(n, 0);
  used[(n - 1)] = 1;
  ans.push_back(a[(n - 1)]);
  var last: dynamic = a[(n - 1)];
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var take: dynamic = cpp_uninitialized();
      var maks: dynamic = -1;
      var cek: dynamic = cpp_uninitialized();
      {
        var j: dynamic = (n - 1);
        while ((j >= 0))
        {
          if ((used[j] == 0))
          {
            cek = gcd(last, a[j]);
            if ((maks < cek))
            {
              maks = cek;
              take = j;
            }
          }
          j -= 1;
        }
      }
      last = maks;
      ans.push_back(a[take]);
      used[take] = 1;
      i += 1;
    }
  }
  for (var i: dynamic in ans)
  {
    write(i, " ");
  }
  write("\n");
}

func main() -> dynamic
{
  var tc: dynamic = 1;
  var num: dynamic = 0;
  read(tc);
  while (cpp_update(tc, "--"))
  {
    num += 1;
    solve(num);
  }
  return 0;
}
