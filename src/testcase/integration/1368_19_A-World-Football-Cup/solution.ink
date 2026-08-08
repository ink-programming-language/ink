// Translated from solution.cpp.

var Maxn = (50 + 10);

var n: dynamic;

class tim
{
  var scr: dynamic;
  var goal: dynamic;
  var sub: dynamic;
  var name: dynamic;
}

var arr = cpp_array(Maxn);

var ans: dynamic;

func moq(a: dynamic, b: dynamic)
{
  if ((a.scr < b.scr))
  {
    return true;
  }
  if ((a.scr > b.scr))
  {
    return false;
  }
  if ((a.sub < b.sub))
  {
    return true;
  }
  if ((a.sub > b.sub))
  {
    return false;
  }
  if ((a.goal < b.goal))
  {
    return true;
  }
  return false;
}

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i].name);
      i += 1;
    }
  }
  {
    var k = 0;
    while ((k < ((n * ((n - 1))) / 2)))
    {
      var s1: dynamic;
      var s2: dynamic;
      var s: dynamic;
      var sp: dynamic;
      read(s, sp);
      var j = 0;
      while ((s[j] != cpp_char("-")))
      {
        s1 += s[j];
        j += 1;
      }
      j += 1;
      while ((j < s.size()))
      {
        s2 += s[j];
        j += 1;
      }
      var g1 = 0;
      var g2 = 0;
      j = 0;
      while ((sp[j] != cpp_char(":")))
      {
        g1 *= 10;
        g1 += (sp[j] - cpp_char("0"));
        j += 1;
      }
      j += 1;
      while ((j < sp.size()))
      {
        g2 *= 10;
        g2 += (sp[j] - cpp_char("0"));
        j += 1;
      }
      {
        var i = 0;
        while ((i < n))
        {
          if ((arr[i].name == s1))
          {
            arr[i].goal += g1;
            arr[i].sub += (g1 - g2);
            if ((g1 > g2))
            {
              arr[i].scr += 3;
            } else
            {
              if ((g1 == g2))
              {
                arr[i].scr += 1;
              }
            }
          }
          if ((arr[i].name == s2))
          {
            arr[i].goal += g2;
            arr[i].sub += (g2 - g1);
            if ((g2 > g1))
            {
              arr[i].scr += 3;
            } else
            {
              if ((g1 == g2))
              {
                arr[i].scr += 1;
              }
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  sort(arr, (arr + n), moq);
  {
    var i = (n / 2);
    while ((i < n))
    {
      ans.push_back(arr[i].name);
      i += 1;
    }
  }
  sort(ans.begin(), ans.end());
  {
    var i = 0;
    while ((i < ans.size()))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
