// Translated from solution.cpp.

var h: dynamic;

var w: dynamic;

var s = cpp_array(50);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var di: dynamic;
  var dj: dynamic;
  read(h, w);
  {
    i = 0;
    while ((i < h))
    {
      read(s[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < h))
    {
      {
        j = 0;
        while ((j < w))
        {
          if ((s[i][j] == cpp_char("#")))
          {
            write(cpp_char("#"));
            j += 1;
            continue;
          }
          var cnt = 0;
          {
            di = -1;
            while ((di <= 1))
            {
              {
                dj = -1;
                while ((dj <= 1))
                {
                  if ((((((0 <= (i + di)) && ((i + di) < h)) && (0 <= (j + dj))) && ((j + dj) < w)) && (s[(i + di)][(j + dj)] == cpp_char("#"))))
                  {
                    cnt += 1;
                  }
                  dj += 1;
                }
              }
              di += 1;
            }
          }
          write(cnt);
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
