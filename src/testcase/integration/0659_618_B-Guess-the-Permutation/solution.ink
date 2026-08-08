// Translated from solution.cpp.

func main()
{
  var matrix = cpp_array(52);
  var n: dynamic;
  var fila = cpp_construct(52, 0);
  read(n);
  {
    var i = int_cpp(0);
    while ((i < int_cpp(n)))
    {
      {
        var j = int_cpp(0);
        while ((j < int_cpp(n)))
        {
          var num: dynamic;
          read(num);
          matrix[i].push_back(num);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = int_cpp(0);
    while ((i < int_cpp(n)))
    {
      var rep = cpp_construct(51, 0);
      {
        var j = int_cpp(0);
        while ((j < int_cpp(n)))
        {
          rep[matrix[i][j]] += 1;
          j += 1;
        }
      }
      var pos = 0;
      var maxi = 0;
      {
        var j = int_cpp(1);
        while ((j < int_cpp(n)))
        {
          if ((maxi < rep[j]))
          {
            pos = j;
            maxi = rep[j];
          }
          j += 1;
        }
      }
      fila[i] = pos;
      {
        var j = int_cpp(0);
        while ((j < int_cpp(n)))
        {
          if ((matrix[i][j] == j))
          {
            (matrix[i][j] == 0);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var pos1 = -1;
  var pos2: dynamic;
  {
    var i = int_cpp(0);
    while ((i < int_cpp(n)))
    {
      {
        var j = int_cpp(0);
        while ((j < int_cpp(n)))
        {
          if ((matrix[i][j] == (n - 1)))
          {
            if ((pos1 == -1))
            {
              pos1 = i;
            } else
            {
              pos2 = i;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = int_cpp(0);
    while ((i < int_cpp(n)))
    {
      if ((i == pos1))
      {
        pos1 = -1;
        write((n - 1), " ");
      } else if ((pos2 == i))
      {
        write(n, " ");
      } else
      {
        write(fila[i], " ");
      }
      i += 1;
    }
  }
}
