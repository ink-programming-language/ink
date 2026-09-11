// Translated from solution.cpp.

func mini(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.size() < b.size()))
  {
    return a;
  }
  if ((a.size() > b.size()))
  {
    return b;
  }
  {
    var c: dynamic = 0;
    while ((c < cpp_cast(a.size())))
    {
      if ((a[c] < b[c]))
      {
        return a;
      }
      if ((a[c] > b[c]))
      {
        return b;
      }
      c += 1;
    }
  }
  return a;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var prev: dynamic = "0";
  var ans: dynamic = cpp_uninitialized();
  while (cpp_update(n, "--"))
  {
    var num: dynamic = cpp_uninitialized();
    read(num);
    if ((prev.size() > num.size()))
    {
      write("NO\n");
      exit(0);
    } else if ((num.size() > prev.size()))
    {
      {
        var c: dynamic = 0;
        while ((c < num.size()))
        {
          if ((num[c] == cpp_char("?")))
          {
            num[c] = ((cpp_char("0") + ((c == 0))));
          }
          c += 1;
        }
      }
    } else
    {
      var gg: dynamic = "99999999999";
      if (((mini(num, prev) == prev) && (num != prev)))
      {
        var c_q: dynamic = 0;
        for (var cym: dynamic in num)
        {
          c_q += ((cym == cpp_char("?")));
        }
        if ((c_q == 0))
        {
          gg = num;
        }
      }
      {
        var c: dynamic = 0;
        while ((c < num.size()))
        {
          if (((((prev[c] != cpp_char("9")) && (num[c] == cpp_char("?")))) || (((num[c] != cpp_char("?")) && (prev[c] < num[c])))))
          {
            var num2: dynamic = num;
            if (((num2[c] == cpp_char("?")) && (prev[c] != cpp_char("9"))))
            {
              num2[c] = cpp_cast(((prev[c] + 1)));
            }
            var can: dynamic = true;
            {
              var c0: dynamic = 0;
              while ((c0 < c))
              {
                if (((num[c0] == cpp_char("?")) || (num[c0] == prev[c0])))
                {
                  num2[c0] = prev[c0];
                } else
                {
                  can = false;
                }
                c0 += 1;
              }
            }
            {
              var c1: dynamic = (c + 1);
              while ((c1 < num.size()))
              {
                if ((num2[c1] == cpp_char("?")))
                {
                  num2[c1] = cpp_char("0");
                }
                c1 += 1;
              }
            }
            if ((can && (((mini(num2, prev) == prev) && (prev != num2)))))
            {
              gg = mini(gg, num2);
            }
          }
          c += 1;
        }
      }
      if ((gg.size() > 10))
      {
        write("NO\n");
        exit(0);
      }
      num = gg;
    }
    ans.push_back(num);
    prev = num;
  }
  write("YES\n");
  for (var s: dynamic in ans)
  {
    write(s, cpp_char("\n"));
  }
}
