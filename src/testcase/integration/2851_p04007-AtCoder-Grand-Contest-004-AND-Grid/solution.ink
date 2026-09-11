// Translated from solution.cpp.

var map: dynamic = cpp_array(500, 500);

var ans1: dynamic = cpp_array(500, 500);

func main() -> dynamic
{
  var mx: dynamic = cpp_uninitialized();
  var my: dynamic = cpp_uninitialized();
  scanf("%d%d", (&mx), (&my));
  {
    var i: dynamic = 0;
    while ((i < mx))
    {
      {
        var j: dynamic = 0;
        while ((j < my))
        {
          var z: dynamic = cpp_uninitialized();
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
    var i: dynamic = 0;
    while ((i < mx))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < mx))
    {
      {
        var j: dynamic = 0;
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
