// Translated from solution.cpp.

var big = pow(10, 13);

func main()
{
  var n: dynamic;
  read(n);
  var s = cpp_new();
  var c = cpp_new();
  var mp = cpp_new();
  {
    var i = 0;
    while ((i < n))
    {
      read(s[i]);
      mp[i] = big;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(c[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      {
        var t = (i - 1);
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
  var ans = big;
  var flag = 0;
  {
    var k = 2;
    while ((k < n))
    {
      {
        var j = (k - 1);
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
