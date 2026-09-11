// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(50);

var x: dynamic = cpp_uninitialized();

var b: dynamic = cpp_array(50);

var f: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  i = 1;
  while ((n > 0))
  {
    a[i] = (n % 10);
    n = (n / 10);
    i += 1;
  }
  sort((a + 1), (a + i));
  if ((a[1] == 0))
  {
    {
      j = 2;
      while ((j < i))
      {
        if ((a[j] > 0))
        {
          swap(a[1], a[j]);
          break;
        }
        j += 1;
      }
    }
  }
  gets(b);
  gets(b);
  x = strlen(b);
  x -= 1;
  j = 0;
  f = true;
  while ((j <= x))
  {
    if ((((j == 0) && (j < x)) && (b[j] == 48)))
    {
      f = false;
      break;
    }
    m = (((m * 10) + b[j]) - 48);
    j += 1;
  }
  if ((!f))
  {
    printf("WRONG_ANSWER");
    return 0;
  }
  if (((n == 0) && (m == 0)))
  {
    printf("OK");
    return 0;
  }
  i -= 1;
  n = 0;
  j = 1;
  while ((j <= i))
  {
    n = ((n * 10) + a[j]);
    j += 1;
  }
  if ((m != n))
  {
    printf("WRONG_ANSWER");
    return 0;
  }
  printf("OK");
  return 0;
}
