// Translated from solution.cpp.

var s = cpp_array(1050);

var ans = cpp_array(1050);

var fl = cpp_array(1050);

var c = cpp_array(1050);

func is_prime(n: dynamic)
{
  var sqd = n;
  sqd = sqrt(sqd);
  var sq = sqd;
  sq += 1;
  {
    var i = 2;
    while ((i <= sq))
    {
      if ((((n % i)) == 0))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var test: dynamic;
  var t = 1;
  scanf("%s", s);
  var len = strlen(s);
  var now = 1;
  fl[0] = 1;
  j = (len / 2);
  if (j)
  {
    {
      i = (j + 1);
      while ((i <= len))
      {
        if (is_prime(i))
        {
          fl[(i - 1)] = 1;
          now += 1;
        }
        i += 1;
      }
    }
  }
  var req = (len - now);
  {
    i = 0;
    while (s[i])
    {
      c[(s[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  var flag = 0;
  var mark = 0;
  {
    i = 0;
    while ((i < 26))
    {
      if ((c[i] >= req))
      {
        mark = i;
        flag = 1;
        c[i] -= req;
        break;
      }
      i += 1;
    }
  }
  if ((!flag))
  {
    printf("NO\n");
    return 0;
  }
  j = 0;
  {
    i = 0;
    while ((i < len))
    {
      if ((!fl[i]))
      {
        ans[i] = (mark + cpp_char("a"));
      } else
      {
        while ((!c[j]))
        {
          j += 1;
        }
        ans[i] = (j + cpp_char("a"));
        c[j] -= 1;
      }
      i += 1;
    }
  }
  ans[len] = 0;
  printf("YES\n%s\n", ans);
  return 0;
}
