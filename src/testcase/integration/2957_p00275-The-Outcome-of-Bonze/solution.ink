// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_array(101);
  while (cpp_comma(scanf("%d", (&N)), N))
  {
    scanf("%s", S);
    var H: dynamic = [0];
    var F: dynamic = 0;
    var T: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < 100))
      {
        if ((S[i] == cpp_char("S")))
        {
          F += (H[T] + 1);
          H[T] = 0;
        } else if ((S[i] == cpp_char("M")))
        {
          H[T] += 1;
        } else
        {
          H[T] += (F + 1);
          F = 0;
        }
        if ((T == (N - 1)))
        {
          T = 0;
        } else
        {
          T += 1;
        }
        i += 1;
      }
    }
    sort(H, (H + N));
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        printf("%d ", H[i]);
        i += 1;
      }
    }
    printf("%d\n", F);
  }
}
