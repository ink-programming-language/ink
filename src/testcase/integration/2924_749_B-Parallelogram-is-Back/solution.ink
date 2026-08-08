// Translated from solution.cpp.

func main()
{
  var x = cpp_array(3);
  var y = cpp_array(3);
  var midx = cpp_array(3);
  var midy = cpp_array(3);
  var finx = cpp_array(3);
  var finy = cpp_array(3);
  {
    var i = 0;
    while ((i < 3))
    {
      read(x[i], y[i]);
      i += 1;
    }
  }
  midx[0] = (((x[0] + x[1])) / 2);
  midy[0] = (((y[0] + y[1])) / 2);
  midx[1] = (((x[1] + x[2])) / 2);
  midy[1] = (((y[1] + y[2])) / 2);
  midx[2] = (((x[2] + x[0])) / 2);
  midy[2] = (((y[2] + y[0])) / 2);
  finx[0] = ((2 * midx[0]) - x[2]);
  finy[0] = ((2 * midy[0]) - y[2]);
  finx[1] = ((2 * midx[1]) - x[0]);
  finy[1] = ((2 * midy[1]) - y[0]);
  finx[2] = ((2 * midx[2]) - x[1]);
  finy[2] = ((2 * midy[2]) - y[1]);
  write(3, "\n");
  {
    var i = 0;
    while ((i < 3))
    {
      write(finx[i], " ", finy[i], "\n");
      i += 1;
    }
  }
  return 0;
}
