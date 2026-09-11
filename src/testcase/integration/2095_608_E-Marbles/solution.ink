// Translated from solution.cpp.

var N: dynamic = (1e6 + 100);

var n: dynamic = cpp_uninitialized();

var s1: dynamic = cpp_array(N);

var s2: dynamic = cpp_array(N);

var len: dynamic = cpp_uninitialized();

var Next: dynamic = cpp_array(N);

func getnext(s: dynamic) -> dynamic
{
  Next[0] = -1;
  var j: dynamic = -1;
  var i: dynamic = 0;
  while ((i < n))
  {
    if (((j == -1) || (s[i] == s[j])))
    {
      i += 1;
      j += 1;
      Next[i] = j;
    } else
    {
      j = Next[j];
    }
  }
}

func kmp(s1: dynamic, s2: dynamic) -> dynamic
{
  var i: dynamic = 0;
  var j: dynamic = 0;
  while ((i < n))
  {
    if (((j == -1) || (s1[i] == s2[j])))
    {
      i += 1;
      j += 1;
    } else
    {
      j = Next[j];
    }
  }
  if ((j > 0))
  {
    return 1;
  }
  return 0;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  scanf("%s%s", s1, s2);
  n -= 1;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s1[i] == cpp_char("E")))
      {
        s1[i] = cpp_char("W");
      } else if ((s1[i] == cpp_char("W")))
      {
        s1[i] = cpp_char("E");
      } else if ((s1[i] == cpp_char("N")))
      {
        s1[i] = cpp_char("S");
      } else if ((s1[i] == cpp_char("S")))
      {
        s1[i] = cpp_char("N");
      }
      i += 1;
    }
  }
  reverse(s1, (s1 + n));
  getnext(s1);
  if (kmp(s2, s1))
  {
    puts("NO");
  } else
  {
    puts("YES");
  }
}
