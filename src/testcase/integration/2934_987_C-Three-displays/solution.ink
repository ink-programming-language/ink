// Translated from solution.cpp.

var big: dynamic = pow(10, 13);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_new();
  var c: dynamic = cpp_new();
  var mp: dynamic = cpp_new();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(s[i]);
      mp[i] = big;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(c[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      {
        var t: dynamic = (i - 1);
        while ((t >= 0))
        {
          if ((s[i] > s[t]))
          {
            mp[i] = min(mp[i], c[t]);
          }
          t -= 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = big;
  var flag: dynamic = 0;
  {
    var k: dynamic = 2;
    while ((k < n))
    {
      {
        var j: dynamic = (k - 1);
        while ((j >= 0))
        {
          if (((s[k] > s[j]) && (mp[j] != big)))
          {
            flag = 1;
            ans = min(((c[k] + c[j]) + mp[j]), ans);
          }
          j -= 1;
        }
      }
      k += 1;
    }
  }
  if ((flag == 0))
  {
    write(-1, "\n");
    return 0;
  }
  write(ans, "\n");
}
