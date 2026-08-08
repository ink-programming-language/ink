// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var a: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  read(a);
  if (((a % 2) == 0))
  {
    write((a / 2), "\n");
    {
      var i = 0;
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
      var i = 0;
      while ((i < (((a - 3)) / 2)))
      {
        write("2 ");
        i += 1;
      }
    }
    write(3);
  }
}
