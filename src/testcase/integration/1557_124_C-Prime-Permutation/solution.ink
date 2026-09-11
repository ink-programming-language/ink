// Translated from solution.cpp.

var s: dynamic = cpp_array(1050);

var ans: dynamic = cpp_array(1050);

var fl: dynamic = cpp_array(1050);

var c: dynamic = cpp_array(1050);

func is_prime(n: dynamic) -> dynamic
{
  var sqd: dynamic = n;
  sqd = sqrt(sqd);
  var sq: dynamic = sqd;
  sq += 1;
  {
    var i: dynamic = 2;
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var test: dynamic = cpp_uninitialized();
  var t: dynamic = 1;
  scanf("%s", s);
  var len: dynamic = strlen(s);
  var now: dynamic = 1;
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
  var req: dynamic = (len - now);
  {
    i = 0;
    while (s[i])
    {
      c[(s[i] - cpp_char("a"))] += 1;
      i += 1;
    }
  }
  var flag: dynamic = 0;
  var mark: dynamic = 0;
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
