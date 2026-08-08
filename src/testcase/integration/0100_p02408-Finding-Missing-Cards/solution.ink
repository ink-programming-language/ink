// Translated from solution.cpp.

func main()
{
  var mark = "SHCD";
  var card = [0];
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      var c: dynamic;
      var t: dynamic;
      scanf(" %c %d", (&c), (&t));
      card[mark.find(c)][(t - 1)] = true;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 4))
    {
      {
        var j = 0;
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
