// Translated from solution.cpp.

var in_cpp: dynamic = cpp_array(1000005);

var st: dynamic = cpp_array(1000005);

var val: dynamic = cpp_array(1000005);

var cst: dynamic = cpp_uninitialized();

var maxx: dynamic = 0;

var ct: dynamic = 1;

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%s", (in_cpp + 1));
  {
    i = 1;
    while ((in_cpp[i] != cpp_char("\u{0}")))
    {
      if ((in_cpp[i] == cpp_char("(")))
      {
        st[cpp_update(cst, "++")] = i;
      } else if ((cst > 0))
      {
        cst -= 1;
        val[i] = (((i - st[cst]) + 1) + val[(st[cst] - 1)]);
        if ((val[i] > maxx))
        {
          maxx = val[i];
          ct = 1;
        } else if ((val[i] == maxx))
        {
          ct += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d %d\n", maxx, ct);
}
