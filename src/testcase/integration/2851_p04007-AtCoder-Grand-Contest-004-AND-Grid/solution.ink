// Translated from solution.cpp.

var map = cpp_array(500, 500);

var ans1 = cpp_array(500, 500);

func main()
{
  var mx: dynamic;
  var my: dynamic;
  scanf("%d%d", (&mx), (&my));
  {
    var i = 0;
    while ((i < mx))
    {
      {
        var j = 0;
        while ((j < my))
        {
          var z: dynamic;
          scanf(" %c", (&z));
          if ((z == cpp_char("#")))
          {
            map[i][j] = cpp_assign(ans1[i][j], "=", 1);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < mx))
    {
      {
        var j = 0;
        while ((j < my))
        {
          if ((((i == 0) || ((((((0 != j) && (j != (my - 1))) && (0 != i)) && (i != (mx - 1))) && ((j % 2) == 0)))) || map[i][j]))
          {
            printf("#");
          } else
          {
            printf(".");
          }
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
  printf("\n");
  {
    var i = 0;
    while ((i < mx))
    {
      {
        var j = 0;
        while ((j < my))
        {
          if ((((i == (mx - 1)) || ((((((0 != j) && (j != (my - 1))) && (0 != i)) && (i != (mx - 1))) && ((j % 2) == 1)))) || map[i][j]))
          {
            printf("#");
          } else
          {
            printf(".");
          }
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
}
