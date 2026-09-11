// Translated from solution.cpp.

func main() -> dynamic
{
  var mark: dynamic = "SHCD";
  var card: dynamic = [0];
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var c: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf(" %c %d", (&c), (&t));
      card[mark.find(c)][(t - 1)] = true;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      {
        var j: dynamic = 0;
        while ((j < 13))
        {
          if ((!card[i][j]))
          {
            printf("%c %d\n", mark[i], (j + 1));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
