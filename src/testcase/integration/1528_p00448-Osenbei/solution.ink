// Translated from solution.cpp.

func Rep(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < N; i++)");
}

func main() -> dynamic
{
  var H: dynamic = cpp_uninitialized();
  var W: dynamic = cpp_uninitialized();
  var mas: dynamic = cpp_array(10005, 15);
  while (cpp_comma(((cin >> H) >> W), (W || H)))
  {
    cpp_statement("Rep(i, H) Rep(j, W) cin >> mas[i][j]; int maxv = 0; Rep(i, 1 << H)");
    {
      var cnt: dynamic = 0;
      maxv = max(maxv, cnt);
    }
    write(maxv, "\n");
  }
  return 0;
}

func Rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((((i >> k)) & 1))
          {
            cnts += (1 - mas[k][j]);
          } else
          {
            cnts += mas[k][j];
          }
        }

func Rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var cnts: dynamic = 0;
        cnt += max(cnts, (H - cnts));
      }
