// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var a: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  read(a);
  if (((a % 2) == 0))
  {
    write((a / 2), "\n");
    {
      var i: dynamic = 0;
      while ((i < (a / 2)))
      {
        write("2 ");
        i += 1;
      }
    }
  } else
  {
    write(((((a - 3)) / 2) + 1), "\n");
    {
      var i: dynamic = 0;
      while ((i < (((a - 3)) / 2)))
      {
        write("2 ");
        i += 1;
      }
    }
    write(3);
  }
}
