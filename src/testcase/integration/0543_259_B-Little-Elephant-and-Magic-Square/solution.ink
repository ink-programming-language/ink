// Translated from solution.cpp.

var cell = cpp_array(4, 4);

func go()
{
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          write(cell[i][j], " ");
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
}

func main()
{
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          read(cell[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  cell[0][0] = (((cell[1][2] + cell[2][1])) / 2);
  cell[2][2] = (((cell[0][1] + cell[1][0])) / 2);
  cell[1][1] = ((((cell[0][0] + cell[0][2])) - cell[2][1]));
  go();
  return 0;
}
