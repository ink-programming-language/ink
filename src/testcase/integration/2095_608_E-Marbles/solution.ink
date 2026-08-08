// Translated from solution.cpp.

var N = (1e6 + 100);

var n: dynamic;

var s1 = cpp_array(N);

var s2 = cpp_array(N);

var len: dynamic;

var Next = cpp_array(N);

func getnext(s: dynamic)
{
  Next[0] = -1;
  var j = -1;
  var i = 0;
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

func kmp(s1: dynamic, s2: dynamic)
{
  var i = 0;
  var j = 0;
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

func main()
{
  scanf("%d", (&n));
  scanf("%s%s", s1, s2);
  n -= 1;
  {
    var i = 0;
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
