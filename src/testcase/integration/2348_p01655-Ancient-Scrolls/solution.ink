// Translated from solution.cpp.

var MOD: dynamic = 998244353;

var EPS: dynamic = 1e-9;

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

func func_cpp(asum: dynamic, bsum: dynamic, csum: dynamic, sum: dynamic) -> dynamic
{
  if ((((asum > M) || (bsum > M)) || (csum > M)))
  {
    return false;
  }
  var aadd: dynamic = sum[3];
  var badd: dynamic = sum[2];
  var cadd: dynamic = sum[1];
  if (((asum + aadd) > M))
  {
    var box: dynamic = ((asum + aadd) - M);
    aadd -= box;
    badd += box;
    cadd += box;
  }
  if (((bsum + badd) > M))
  {
    var box: dynamic = ((bsum + badd) - M);
    badd -= box;
    aadd += box;
    cadd += box;
  }
  if (((csum + cadd) > M))
  {
    var box: dynamic = ((csum + cadd) - M);
    cadd -= box;
    badd += box;
    aadd += box;
  }
  aadd += sum[4];
  badd += sum[4];
  cadd += sum[4];
  var amari: dynamic = sum[4];
  if (((asum + aadd) > M))
  {
    var box: dynamic = min(amari, ((asum + aadd) - cpp_cast(M)));
    aadd -= box;
    amari -= box;
  }
  if (((bsum + badd) > M))
  {
    var box: dynamic = min(amari, ((bsum + badd) - cpp_cast(M)));
    badd -= box;
    amari -= box;
  }
  if (((csum + cadd) > M))
  {
    var box: dynamic = min(amari, ((csum + cadd) - cpp_cast(M)));
    cadd -= box;
    amari -= box;
  }
  return ((((asum + aadd) <= M) && ((csum + cadd) <= M)) && ((bsum + badd) <= M));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  while (cpp_comma(((cin >> N) >> M), N))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    read(a, b, c);
    var sum: dynamic = cpp_construct(5);
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        if (((a[i] == b[i]) && (a[i] != c[i])))
        {
          num[i] = 1;
        }
        if (((a[i] == c[i]) && (a[i] != b[i])))
        {
          num[i] = 2;
        }
        if (((b[i] == c[i]) && (b[i] != a[i])))
        {
          num[i] = 3;
        }
        if ((((a[i] != b[i]) && (a[i] != c[i])) && (b[i] != c[i])))
        {
          num[i] = 4;
        }
        sum[num[i]] += 1;
        i += 1;
      }
    }
    var ans: dynamic = "-1";
    var ret: dynamic = cpp_uninitialized();
    var asum: dynamic = 0;
    var bsum: dynamic = 0;
    var csum: dynamic = 0;
    var check: dynamic = cpp_uninitialized();
    {
      var i: dynamic = cpp_char("A");
      while ((i <= cpp_char("Z")))
      {
        check.push_back(i);
        i += 1;
      }
    }
    {
      var i: dynamic = cpp_char("a");
      while ((i <= cpp_char("z")))
      {
        check.push_back(i);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        sum[num[i]] -= 1;
        for (var j: dynamic in check)
        {
          asum += (a[i] != j);
          bsum += (b[i] != j);
          csum += (c[i] != j);
          if (func_cpp(asum, bsum, csum, sum))
          {
            ret.push_back(j);
            break;
          } else
          {
            asum -= (a[i] != j);
            bsum -= (b[i] != j);
            csum -= (c[i] != j);
          }
        }
        i += 1;
      }
    }
    if ((ret.size() == N))
    {
      ans = ret;
    }
    write(ans, "\n");
  }
}
