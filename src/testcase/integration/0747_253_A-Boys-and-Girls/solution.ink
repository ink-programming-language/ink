// Translated from solution.cpp.

func solve() -> dynamic
{
  var in_cpp: dynamic = cpp_construct("input.txt");
  var out: dynamic = cpp_construct("output.txt");
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  ((in_cpp >> n) >> m);
  if ((n >= m))
  {
    {
      int_cpp(i) = int_cpp(0);
      while (((i) < int_cpp(m)))
      {
        (out << "BG");
        (i) += 1;
      }
    }
    {
      int_cpp(i) = int_cpp(0);
      while (((i) < int_cpp((n - m))))
      {
        (out << "B");
        (i) += 1;
      }
    }
  } else
  {
    {
      int_cpp(i) = int_cpp(0);
      while (((i) < int_cpp(n)))
      {
        (out << "GB");
        (i) += 1;
      }
    }
    {
      int_cpp(i) = int_cpp(0);
      while (((i) < int_cpp((m - n))))
      {
        (out << "G");
        (i) += 1;
      }
    }
  }
  write("\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  solve();
  return 0;
}
