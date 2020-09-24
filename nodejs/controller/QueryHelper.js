export function runAsync(sql, params, db) {
    return new Promise(function (resolve, reject) {
        db.run(sql, params, function (err) {
            if (err)
                reject(err);
            else
                resolve(this.lastID);
        });
    })
}

export function allAsync(sql, params, db) {
    return new Promise(function (resolve, reject) {
        db.all(sql, params, function (err, rows) {
            if (err)
                reject(err);
            else
                resolve(rows);
        });
    });
}

export function getAsync(sql, params, db) {
    return new Promise(function (resolve, reject) {
        db.get(sql, params, function (err, row) {
            if (err)
                reject(err);
            else
                resolve(row);
        });
    });
}
