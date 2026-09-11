// Translated from solution.cpp.

var N: dynamic = 1000001;

func highp2(n: dynamic) -> dynamic
{
  return ((n & ((~((n - 1))))));
}

func isPrime(n: dynamic) -> dynamic
{
  if ((n <= 1))
  {
    return false;
  }
  if ((n <= 3))
  {
    return true;
  }
  if ((((n % 2) == 0) || ((n % 3) == 0)))
  {
    return false;
  }
  {
    var i: dynamic = 5;
    while (((i * i) <= n))
    {
      if ((((n % i) == 0) || ((n % ((i + 2))) == 0)))
      {
        return false;
      }
      i = (i + 6);
    }
  }
  return true;
}

func binarysearchlf(l: dynamic, h: dynamic, a: dynamic, k: dynamic) -> dynamic
{
  while ((l < h))
  {
    var mid: dynamic = (l + (((((h - l) + 1)) / 2)));
    if ((a[mid] < k))
    {
      l = mid;
    } else
    {
      h = (mid - 1);
    }
  }
  return l;
}

func binarysearchft(l: dynamic, h: dynamic, a: dynamic, k: dynamic) -> dynamic
{
  while ((l < h))
  {
    var mid: dynamic = (l + ((((h - l)) / 2)));
    if ((a[mid] < k))
    {
      l = (mid + 1);
    } else
    {
      h = mid;
    }
  }
  return l;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    swap(a, b);
  }
  if ((b == 0))
  {
    return a;
  }
  return (cpp_comma(b, (a % b)));
}

func getstring(k: dynamic, x: dynamic) -> dynamic
{
  return s;
}

func sort1(a: dynamic, b: dynamic) -> dynamic
{
  return (a.second < b.second);
}

func sort2(v1: dynamic, v2: dynamic) -> dynamic
{
  if ((v1[1] > v2[1]))
  {
    return true;
  } else if ((v1[1] < v2[1]))
  {
    return false;
  } else
  {
    return (v1[2] > v2[2]);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var i1: dynamic = cpp_uninitialized();
  var i2: dynamic = cpp_uninitialized();
  var flag1: dynamic = 0;
  var flag2: dynamic = 0;
  var n: dynamic = s.length();
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      if (((s[i] == cpp_char("A")) && (s[(i + 1)] == cpp_char("B"))))
      {
        i1 = i;
        flag1 = 1;
        break;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      if (((s[i] == cpp_char("B")) && (s[(i + 1)] == cpp_char("A"))))
      {
        i2 = i;
        flag2 = 1;
        break;
      }
      i += 1;
    }
  }
  if (((flag1 == 0) || (flag2 == 0)))
  {
    write("NO", "\n");
    return 0;
  }
  if ((abs((i1 - i2)) > 1))
  {
    write("YES", "\n");
  } else
  {
    var flag: dynamic = 0;
    {
      var i: dynamic = (i2 + 2);
      while ((i < (n - 1)))
      {
        if (((s[i] == cpp_char("A")) && (s[(i + 1)] == cpp_char("B"))))
        {
          flag = 1;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = (i1 + 2);
      while ((i < (n - 1)))
      {
        if (((s[i] == cpp_char("B")) && (s[(i + 1)] == cpp_char("A"))))
        {
          flag = 1;
        }
        i += 1;
      }
    }
    if (flag)
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
    }
  }
  return 0;
}
