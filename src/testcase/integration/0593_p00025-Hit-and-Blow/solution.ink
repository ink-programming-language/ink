// Translated from solution.cpp.

func main()
{
  var a = cpp_array(4);
  var b = cpp_array(4);
  while (((((((((cin >> a[0]) >> a[1]) >> a[2]) >> a[3]) >> b[0]) >> b[1]) >> b[2]) >> b[3]))
  {
    var n1 = 0;
    var n2 = 0;
    {
      var i = 0;
      while ((i < 4))
      {
        if ((a[i] == b[i]))
        {
          n1 += 1;
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 4))
      {
        {
          var j = 0;
          while ((j < 4))
          {
            if ((i == j))
            {
              j += 1;
              continue;
            }
            if ((a[i] == b[j]))
            {
              n2 += 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(n1, " ", n2, "\n");
  }
}
