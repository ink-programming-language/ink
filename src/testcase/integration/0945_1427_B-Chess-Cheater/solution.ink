// Translated from solution.cpp.

var md: dynamic = (1e9 + 7);

var an: dynamic = cpp_array(310000);

var sn: dynamic = cpp_array(310000);

func main() -> dynamic
{
  var z: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(z);
  while (cpp_update(z, "--"))
  {
    scanf("%d%d", (&n), (&k));
    scanf("%s", sn);
    var flag: dynamic = 1;
    var first: dynamic = cpp_uninitialized();
    var last: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < n))
      {
        if (((sn[i] == cpp_char("W")) && flag))
        {
          flag = 0;
          first = i;
        }
        if ((sn[i] == cpp_char("W")))
        {
          last = i;
        }
        i += 1;
      }
    }
    var ans: dynamic = cpp_uninitialized();
    if (flag)
    {
      ans = max(((k * 2) - 1), 0);
    } else
    {
      var l: dynamic = cpp_uninitialized();
      var bl: dynamic = 0;
      var num: dynamic = 0;
      ans = 1;
      {
        i = (first + 1);
        while ((i <= last))
        {
          if (((bl == 0) && (sn[i] == cpp_char("L"))))
          {
            l = 1;
            bl = 1;
          } else if (((bl == 0) && (sn[i] == cpp_char("W"))))
          {
            ans += 2;
          } else if (((bl == 1) && (sn[i] == cpp_char("L"))))
          {
            l += 1;
          } else
          {
            bl = 0;
            an[cpp_update(num, "++")] = l;
            ans += 1;
          }
          i += 1;
        }
      }
      sort(an, (an + num));
      {
        i = 0;
        while ((i < num))
        {
          if ((k >= an[i]))
          {
            ans += ((an[i] * 2) + 1);
            k -= an[i];
          } else
          {
            ans += (k * 2);
            k = 0;
            break;
          }
          i += 1;
        }
      }
      if ((k > 0))
      {
        ans += (min(k, (n - (((last - first) + 1)))) * 2);
      }
    }
    printf("%d\n", ans);
  }
  return 0;
}
