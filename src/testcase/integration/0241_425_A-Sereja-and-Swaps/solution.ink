// Translated from solution.cpp.

func err(it: dynamic) -> dynamic
{
}

func err(it: dynamic, a: dynamic, args: dynamic...) -> dynamic
{
  write((*it), " = ", a, "\n");
  err(cpp_update(it, "++"), cpp_expand(args));
}

var mod: dynamic = (1e9 + 7);

func powm(a: dynamic, b: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      res = (((res * a)) % mod);
    }
    a = (((a * a)) % mod);
    b >>= 1;
  }
  return res;
}

func sortinrev(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.first > b.first));
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = -1e9;
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      {
        var j: dynamic = (i);
        while ((j < (n)))
        {
          var temp: dynamic = cpp_uninitialized();
          var temp1: dynamic = cpp_uninitialized();
          temp1.clear();
          temp.clear();
          var sum: dynamic = 0;
          {
            var ka: dynamic = 0;
            while ((ka < (i)))
            {
              temp1.push_back(a[ka]);
              ka += 1;
            }
          }
          {
            var ka: dynamic = ((j + 1));
            while ((ka < (n)))
            {
              temp1.push_back(a[ka]);
              ka += 1;
            }
          }
          {
            var ka: dynamic = (i);
            while ((ka < ((j + 1))))
            {
              temp.push_back(a[ka]);
              sum += a[ka];
              ka += 1;
            }
          }
          ans = max(ans, sum);
          sort(temp.begin(), temp.end());
          sort(temp1.begin(), temp1.end());
          reverse(temp1.begin(), temp1.end());
          {
            var ka: dynamic = 0;
            while ((ka < (min([k, cpp_cast(temp1.size()), cpp_cast(temp.size())]))))
            {
              sum -= temp[ka];
              sum += temp1[ka];
              ans = max(ans, sum);
              ka += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  write(fixed, setprecision(15));
  var tc: dynamic = 1;
  while (cpp_update(tc, "--"))
  {
    solve();
  }
  return 0;
}
