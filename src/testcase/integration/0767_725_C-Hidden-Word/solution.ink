// Translated from solution.cpp.

var s: dynamic;

var cur: dynamic;

var cnt = cpp_array(26);

var grid = cpp_array(13, 2);

func construct(x: dynamic)
{
  var row = 0;
  var col = x;
  {
    var i = 0;
    while ((i <= ((26) - 1)))
    {
      grid[row][col] = cur[i];
      if (((!row) && (col == 12)))
      {
        row = 1;
      } else if (((row == 1) && (!col)))
      {
        row = 0;
      } else if (row)
      {
        col -= 1;
      } else
      {
        col += 1;
      }
      i += 1;
    }
  }
}

func ok(x: dynamic)
{
  var row = 0;
  var col = x;
  var dr = [-1, -1, -1, 0, 1, 1, 1, 0];
  var dc = [-1, 0, 1, 1, 1, 0, -1, -1];
  {
    var i = 1;
    while ((i <= 26))
    {
      var flag = false;
      {
        var dd = 0;
        while ((dd <= ((8) - 1)))
        {
          var newr = (row + dr[dd]);
          var newc = (col + dc[dd]);
          if (((((newr >= 0) && (newc >= 0)) && (newr < 2)) && (newc < 13)))
          {
            if ((grid[newr][newc] == s[i]))
            {
              row = newr;
              col = newc;
              flag = true;
              break;
            }
          }
          dd += 1;
        }
      }
      if ((!flag))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func f()
{
  {
    var i = 0;
    while ((i <= ((2) - 1)))
    {
      {
        var j = 0;
        while ((j <= ((13) - 1)))
        {
          write(grid[i][j]);
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
  ios.sync_with_stdio(0);
  read(s);
  {
    var i = 0;
    while ((i <= ((26) - 1)))
    {
      if ((s[i] == s[(i + 1)]))
      {
        write("Impossible");
        return 0;
      }
      i += 1;
    }
  }
  cur = "";
  for (var c in s)
  {
    if ((!cnt[(c - cpp_char("A"))]))
    {
      cnt[(c - cpp_char("A"))] = 1;
      cur += c;
    }
  }
  {
    var i = 0;
    while ((i <= ((13) - 1)))
    {
      construct(i);
      if (ok(i))
      {
        f();
        return 0;
      }
      i += 1;
    }
  }
  write("Impossible");
}
