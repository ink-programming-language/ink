// Translated from solution.cpp.

func fastPow(a: dynamic, p: dynamic)
{
  var res = 1;
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

func gcd(a: dynamic, b: dynamic)
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func solve(num: dynamic)
{
  var n: dynamic;
  read(n);
  var a = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var ans: dynamic;
  var used = cpp_construct(n, 0);
  used[(n - 1)] = 1;
  ans.push_back(a[(n - 1)]);
  var last = a[(n - 1)];
  {
    var i = 1;
    while ((i < n))
    {
      var take: dynamic;
      var maks = -1;
      var cek: dynamic;
      {
        var j = (n - 1);
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
  for (var i in ans)
  {
    write(i, " ");
  }
  write("\n");
}

func main()
{
  var tc = 1;
  var num = 0;
  read(tc);
  while (cpp_update(tc, "--"))
  {
    num += 1;
    solve(num);
  }
  return 0;
}
