// Translated from solution.cpp.

var ans: dynamic = cpp_uninitialized();

var jc: dynamic = cpp_array(4010);

var c: dynamic = cpp_array(4010, 4010);

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i <= 4000))
    {
      c[i][0] = 1;
      {
        j = 1;
        while ((j <= i))
        {
          c[i][j] = (c[(i - 1)][j] + c[(i - 1)][(j - 1)]);
          if ((c[i][j] >= 1000000009))
          {
            c[i][j] -= 1000000009;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  jc[0] = 1;
  {
    i = 1;
    while ((i <= 4000))
    {
      jc[i] = ((jc[(i - 1)] * i) % 1000000009);
      i += 1;
    }
  }
  scanf("%d%d%d", (&n), (&w), (&b));
  {
    i = 2;
    while ((n - i))
    {
      ans = (((ans + (((c[(w - 1)][(i - 1)] * c[(b - 1)][((n - i) - 1)]) % 1000000009) * ((i - 1))))) % 1000000009);
      i += 1;
    }
  }
  ans = ((((ans * jc[w]) % 1000000009) * jc[b]) % 1000000009);
  printf("%d\n", cpp_cast(ans));
  return 0;
}
