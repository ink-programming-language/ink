// Translated from solution.cpp.

func pikachu()
{
}

var t: dynamic;

var n: dynamic;

var a = cpp_array(111);

func main()
{
  pikachu();
  scanf("%hd", (&t));
  var u: dynamic;
  var v: dynamic;
  while (cpp_update(t, "--"))
  {
    scanf("%hd", (&n));
    scanf("%s", (a + 1));
    u = 0;
    v = (n + 1);
    {
      var i = 1;
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
