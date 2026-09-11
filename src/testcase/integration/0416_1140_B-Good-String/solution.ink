// Translated from solution.cpp.

func pikachu() -> dynamic
{
}

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(111);

func main() -> dynamic
{
  pikachu();
  scanf("%hd", (&t));
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  while (cpp_update(t, "--"))
  {
    scanf("%hd", (&n));
    scanf("%s", (a + 1));
    u = 0;
    v = (n + 1);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((a[i] == cpp_char("<")))
        {
          u = i;
        } else
        {
          v = min(v, i);
        }
        i += 1;
      }
    }
    printf("%hd\n", min((v - 1), (n - u)));
  }
}
